#include "lib.h"
#include <iostream>
#include <vector>
#include <string>

//LipaLib - Learning Image Processing Autonomic Library

// Strutkura Segments s�u�y do podmiany warto�ci w obrazie
struct Segments{
	Segments(int n = 0) :swap_from(n), swap_to(n)
	{
	}
	int swap_from; // Zmienna kt�ra b�dzie podmieniona
	int swap_to; // Zmienna na kt�r� b�dzie podmiana
};

// Struktura Point2D s�u�y do przchowania punktu o 2 wsp�rz�dnych x,y
struct Point2D{
	Point2D(int n1 = 0, int n2 = 0) :x(n1), y(n2)
	{
	}
	int x, y;// Wsp�rz�dne punktu
};

// Struktura Card tworzy obiekty przechowuj�ce karty
struct Card{
	Card(int n = 0, std::string s = ".", double n2 = 0) :corner1(n), corner2(n), kolor(s), barwa(s), ratio(n2), value(n)
	{
	}
	Point2D corner1, corner2; // Pola przechowuj�ce punkty rog�w le��cych na przek�tnej karty
	std::string kolor; // Pole przechowuj�ce kolor/znak karty
	std::string barwa; // Pole przechowuj�ce barw� karty
	double ratio; // Pole przechowuj�ce stosunek obwodu do pola
	int value; // Pole przechowuj�ce warto�� karty

	void Print(); // Metoda wpisuj�ca dane karty do konsoli
};

// Metoda wpisuj�ca dane karty do konsoli
void Card::Print(){
	std::cout << "Dane karty: " << value << " " << kolor << std::endl;
}

void rgbTogray(Image3CH& rgbImg, Image1CH& grayImg) // arguments with & because we want to save changes to images after performing funtion
{
	//Check if image sizes are equal
	if (rgbImg.width() == grayImg.width() && rgbImg.height() == grayImg.height())
	{
		for (int i = 0; i < rgbImg.width(); i++) //iterate by rows
			for (int j = 0; j < rgbImg.height(); j++) //iterate by cols
			{
				grayImg(i, j).Intensity() = (rgbImg(i, j).Red() + rgbImg(i, j).Green() + rgbImg(i, j).Blue()) / 3; // I = (1/3)*(R+G+B) (i,j) - (number of row, number of col)
			}
	}
	else
	{
		std::cerr << "Image sizes mismatch" << std::endl; //print error
		return;
	}
}


// Funkcja realizuj�ca filtr kraw�dziowy Sobela
void Sobel(Image1CH& in, Image1CH& out){
	//Tablice przechowuj�ce odpowiednie warto�ci maski dla filtru Sobela
	int xTab[9] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
	int yTab[9] = { 1, 1, 1, 0, 0, 0, -1, -1, -1 };

	// Zmienne przechowuj�ce sum� po x, sum� po y oraz warto�� pola
	double sumx = 0, sumy = 0, value;
	int iter = 0; // Iterator
	// Filtracja kraw�dziowa
	for (int i = 1; i < in.width() - 1; ++i) //Iteracja po rz�dach
		for (int j = 1; j < in.height() - 1; ++j) //Iteracja po kolumnach
		{
			// Zerowanie sum oraz iteratora dla ka�dego pixela oddzielnie
			sumx = 0;
			sumy = 0;
			iter = 0;
			for (int k = 0; k < 3; k++)//Iteracja w obr�bie maski
				for (int l = 0; l < 3; ++l){
					sumx += in(i + k - 1, j + l - 1).I()*xTab[iter]; //Dodanie do pola sumx iloczynu maski z obrazem
					sumy += in(i + k - 1, j + l - 1).I()*yTab[iter]; //Dodanie do pola sumy iloczynu maski z obrazem
					iter++; // Inkremenetacja iteratora
				}
			value = std::abs(sumx) + std::abs(sumy); //Wyzaczenie warto�ci pixela na podstawie sumy modu��w
			if (value > 1)value = 1; // Zabezpieczenie przed zbyt du�� warto�ci�
			out(i, j).I() = value; // Wpisanie do pixela nowej warto�ci 
		}
}

// Prcedura znajduj�ca pr�g binaryzacji
double findTresh(Image1CH src, std::vector<double>& tab){
	for (int i = 0; i < src.width(); ++i){ //Wytworzenie histogramu obrazu
		for (int j = 0; j < src.height(); ++j){
			tab[src(i, j).I() * 255]++;
		}
	}

	double tresh = 0; // Zmienna przechowuj�ca ko�cowy pr�g
	double temp_tresh = 128; // Pr�g pocz�tkowy

	double l_sum = 0, r_sum = 0; // Zmienne przechowuj�ce sum� po lewo i prawo od progu
	double r_mean = 0, l_mean = 0; // �rednie na prawo i lewo od progu
	double l_indeks_sum = 0, r_indeks_sum = 0; // Sumy indeks�w

	while (temp_tresh != tresh){ //Dop�ki tresh jest r�zny od temp_tresh
		tresh = temp_tresh; // Przepisz warto�� temp_tresh do tresh
		for (int i = 0; i < tresh; ++i){ // Wyznacz sum� indeks�w(wyst�pie� danej intensywno�ci * intensywno��) oraz sum� po lewo od progu
			l_sum += tab[i];
			l_indeks_sum += tab[i] * i;
		}
		l_mean = l_indeks_sum / l_sum; // Wyznacz �redni� na podstawie sumy indeks�w i sumy

		for (int i = tresh; i < 256; ++i){ // Wyznacz sum� indeks�w oraz sum� po prawo od progu
			r_sum += tab[i];
			r_indeks_sum += tab[i] * i;
		}
		r_mean = r_indeks_sum / r_sum; // Wyznacz �redni� na podstawie sumy indeks�w i sumy

		temp_tresh = (r_mean + l_mean) / 2; //Wyznacz temp_tresh jako �rednia �redniej prawej i lewej
	}
	tresh = tresh / 255; //Normalizacja progu
	return tresh; // Zwr�� pr�g
}

// Funkcja binaryzuj�ca obraz
void Binary(Image1CH src, Image1CH& out){
	double treshold = 0; // Zmienna przechowuj�ca pr�g
	std::vector<double> histogram; // Vector doubli przechowuj�cy histogram obrazu
	histogram.reserve(256); // Zarezerowanie pami�ci na 256 element�w
	for (int i = 0; i < 256; ++i){ // Stworzenie 256 element�w
		histogram.push_back(0);
	}
	treshold = findTresh(src, histogram); // Wyznaczenie progu
	for (int i = 0; i < src.width(); ++i){ //Iteracja po obrazie
		for (int j = 0; j < src.height(); ++j){
			if (src(i, j).I()>treshold){ // Je�li intensywno�� piksela jest wi�ksza od progu to wpisz 1, je�li nie to 0
				out(i, j).I() = 1;
			}
			else
			{
				out(i, j).I() = 0;
			}
		}
	}
}

int med_temp(double *c, int a, int b)
{
	double e = c[a]; // Element na kt�rego miejsce wskazuje a
	double temp = 0; // Pomocnicza
	while (a < b) // Dop�ki a jest mniejsze od b
	{
		while ((a < b) && (c[b] >= e)) b--; // Dop�ki b>a oraz element na kt�ry wskazuje b>e to dekementacja b
		while ((a < b) && (c[a] < e)) a++; // Dop�ki b>a oraz element na kt�ry wskazuje a < e to inkrementacja a
		if (a < b) // Je�li a mniejsze od b to zamie� miejscami warto�ci na kt�re wskazuj� a oraz b
		{
			temp = c[a];
			c[a] = c[b];
			c[b] = temp;
		}
	}
	return a; // Zwr�� a
}
double medianaHoare(double *c, int size)
{
	//algorytm Hoare'a
	int i = 0; // Iterator wskazuj�cy na pozycj� mediany dzia�aj�cy od pocz�tku
	int j = size - 1; // Iterator pomocniczy dzia�aj�cy od ko�ca
	int w = j / 2; // Iterator wskazuj�cy na �rodek przedzia�u
	int k; // Mediana pomocniczna
	while (i != j) // Dop�ki i r�zne od j
	{
		k = med_temp(c, i, j); // Wpisz do k pozycj� a
		k = k - i + 1;
		if (k >= w) // Je�li pozycja mediany jest wi�ksze lub r�wne �rodkowi to do j przypisz sume i,k - 1
			j = i + k - 1;
		if (k < w) // Je�li k jest mniejsze od w to zmniejsz w o k oraz zwieksz i o k
		{
			w -= k;
			i += k;
		}
	}
	return c[i]; // Zwr�c element �rodkowy
}
void Mediana(Image1CH src, Image1CH& out)
{
	double Tab[25]; // Inicjalizacja tablicy 
	int iter = 0; // Iterator
	double mediana; // Zmienna przechowowuj�ca median�
	for (int i = 2; i < src.width() - 2; ++i){ //Iteracja po obrazie
		for (int j = 2; j < src.height() - 2; ++j){
			iter = 0; // Zerowanie iteratora
			for (int k = 0; k < 5; ++k){ // Iteracja po s�siedztwie
				for (int l = 0; l < 5; ++l){
					Tab[iter] = src(i + k - 2, j + l - 2).I();  // Wpisanie do tablicy warto�ci intensywno�ci
					iter++; // Inkrementacja iteratora
				}
			}
			mediana = medianaHoare(Tab, 25); // Wykonanie procedury wyznaczenia mediany
			out(i, j).I() = mediana; // Wpisanie mediany do piksela
		}
	}
}

// Prcedura zanjduj�ca segment
int findSegment(Image1CH out, int x, int y){
	int value = 0; // zmienna przechowuj�ca warto�� segmentu
	for (int i = x - 5; i < x + 5; ++i){ // iteracja o obszarze <-5,5)
		for (int j = y - 5; j < y + 5; ++j){
			if (out(i, j).I()>0)value = int(out(i, j).I()); // je�li warto�� intensywno�ci jest wi�szka od 0 wpisz do value t� warto��
		}
	}
	return value; // zwr�� value
}

// Prcedura licz�ca obw�d
int circuitCalc(Image1CH out, int x, int y){
	int suma = 0; // suma przechowuje sum� obwodu
	if (out(x - 1, y).I() == 0)suma++; // Je�li piksel obok jest 0 oznacza ze jest kraw�dzi� wliczaj�c� si� do obwodu 
	if (out(x + 1, y).I() == 0)suma++;
	if (out(x, y - 1).I() == 0)suma++;
	if (out(x, y + 1).I() == 0)suma++;
	return suma; // zwr�� sum�
}

// Funkcja por�wnuj�ca ze sob� dwie liczby
void Compare(int& a, int& b, int type){
	// Je�li type=1 to znaczy �e wyr�wnujemy do wi�kszej
	if (type == 1){
		if (a > b){ // Je�li a wi�szke od b wpisz a do b 
			b = a;
		}
		else // Je�li a mniejsze od b wpisz b do a 
		{
			a = b;
		}
	}
	else{ //Je�li type=0 to znaczy �e wyr�wnujemy do mniejszej
		if (a > b){ // Je�li a wi�ksze od b wpisz b do a 
			a = b;
		}
		else{ // Je�li a mniejsze od b wpisz b do a
			b = a;
		}
	}
}

// Funkcja segmentuj�ca znaki w karcie
void Segmentation(Image3CH clr, Image1CH& src, Image1CH& out, Card& card){
	// Sprawdzenie poprawnosci i kolejnosci rogow w karcie
	if (card.corner1.x > card.corner2.x){ // Je�li warto�� x wi�ksza w 1 rogu od 2, podmie� 'x'
		int temp = card.corner1.x; // Przechowuj� tymczasowo x
		card.corner1.x = card.corner2.x;
		card.corner2.x = temp;
	}
	if (card.corner1.y > card.corner2.y){ // Je�li warto�� y wi�ksza w 1 rogu od 2, podmie� 'y'
		int temp = card.corner1.y;// Przechowuj� tymczasowo y
		card.corner1.y = card.corner2.y;
		card.corner2.y = temp;
	}

	//Negatyw w danym segmencie
	for (int i = card.corner1.x; i < card.corner2.x; ++i){ //Iteracja po granicach karty
		for (int j = card.corner1.y; j < card.corner2.y; ++j){
			if (src(i, j).I() == 1) // Je�li piksel jest '1' wpisz 0
			{
				src(i, j).I() = 0;
			}
			else // Je�li piksel jest '0' wpisz 1
			{
				src(i, j).I() = 1;
			}
		}
	}

	int segment_type = 0, iter = 0; // Zmienne przechowuj�ce typ segmentu oraz iterator
	double r_channel = 0, b_channel = 0; // Zmienne przechowuj�ce warto�� kana�u czerwonego oraz niebieskiego
	bool check = true; // Zmienna check s�u��ca do wyznaczenia koloru segmentu
	int neighbours_size = 5; // Zmienna s�u�aca do iteracji po s�siedztwie piksela o rozmiarze 5
	int margin = (neighbours_size / 2); // Margines do uwzgl�dnienia przy warunkach brzegowych
	int counter = 0;
	for (int i = card.corner1.y + margin; i < card.corner2.y - margin; ++i){ // Iteracja po rogach karty
		for (int j = card.corner1.x + margin; j < card.corner2.x - margin; ++j){
			if (src(j, i).I() == 1){ // Je�li dany piksel jest '1' 
				segment_type = findSegment(out, j, i); // Wywo�anie procedury w celu wyznaczenia segmentu w s�siedztwie j,i
				if (segment_type == 0){ // Je�li nie ma segmentu w s�siedztwie
					iter++; // Inkrementacja iteratora segment�w
					out(j, i).I() = iter; // Wpisz iterator jako nowy segment do piksela
				}
				else{ // Je�li w s�siedztwie jest segment
					out(j, i).I() = segment_type; // Wpisz segment do piksela
					counter = 0; //  Wyzeruj licznik
					if (check){ // Je�li check jest prawd� (czyli nie wpisali�my jeszcze warto�ci do kana��w koloru)
						for (int k = 0; k < neighbours_size; ++k){ // Iteracja w s�siedztwie punktu
							for (int l = 0; l < neighbours_size; ++l){
								if (src(j - k + margin, i - l + margin).I() != 0){ //Je�li dany piksel jest obiektem inketementuj licznik
									counter++;
								}
							}
						}
						if (counter == 25){ // Je�li licznik ==25 czyli ca�e s�siedztwo jest obiektem
							r_channel = clr(j, i).R(); // Wpisz warto�� danych kana��w danego piksela z obrazu kolorowego
							b_channel = clr(j, i).B();
							check = false; // Zmie� check na false aby nie wykonywa� tej operacji wi�cej
						}
						counter = 0; // Zeruj licznik
					}
				}
			}
		}
	}
	// Analiza przypadku gdy dla jednego obiektu mamy dwie wartosci segmentu
	// Wektor swaps przechowuje segmenty do podmiany
	std::vector<Segments> swaps;
	Segments segment_to_swap; // Zmiennna s�u��ca do wpisania swapa do wektora
	bool exist = false; // Zmienna sprawdzaj�ca czy dany 'swap' ju� istnieje

	for (int i = card.corner1.y + 1; i < card.corner2.y - 1; ++i){ // Iteracja po granicach karty
		for (int j = card.corner1.x + 1; j < card.corner2.x - 1; ++j){
			// Je�li obecny piksel nie jest zerem, piksel nad nie jest zerem(poniewa� iterujemy rz�dami) oraz nie s� one r�wne
			if (out(j, i).I() != 0 && out(j, i - 1).I() != 0 && out(j, i).I() != out(j, i - 1).I()){
				exist = false; // Do exist wpisz false
				for (auto s : swaps){ // Iteracja po wszystkich elementach wektora swaps
					if (s.swap_from == out(j, i - 1).I())exist = true; // Je�li istnieje ju� dany swap wpisz do exist true
				}
				if (!exist){ // Je�li taki swap nie istenieje tworzymy nowy swap i wpisujemy go do wektora
					segment_to_swap.swap_from = out(j, i - 1).I();
					segment_to_swap.swap_to = out(j, i).I();
					swaps.push_back(segment_to_swap);
				}
			}
		}
	}

	// Przyporz�dkowanie ka�demu segmentowi jednej zmiennej
	for (int i = card.corner1.y + 1; i < card.corner2.y - 1; ++i){ //Iteracja po granicach karty
		for (int j = card.corner1.x + 1; j < card.corner2.x - 1; ++j){
			if (out(j, i).I() != 0){ // Je�li dany piksel nie jest zerem 
				for (auto p : swaps){ // Iteracja po wszystkich elementach wektora swaps
					if (p.swap_from == out(j, i).I())out(j, i).I() = p.swap_to; // Je�li dana intensywno�� piksela zgadza si� z jednym ze swap�w przepisz mu odpowiedni� warto��
				}
			}
		}
	}
	// Sprawdzenie jakiej barwy s� figury na karcie
	if (r_channel - b_channel > 0.15)
	{
		card.barwa = "Czerwony";
	}
	else
	{
		card.barwa = "Czarny";
	}
	// Warto�� karty jest to ilo�� wprowadzonych segment�w minus 4 figury na rogach oraz liczba zmian
	card.value = iter - swaps.size() - 4;

	int value = 0, obwod = 0, pole = 0;
	// Wyznaczenie punktu �rodka karty
	Point2D Center((card.corner1.x + card.corner2.x) / 2, (card.corner1.y + card.corner2.y) / 2);

	// Wyznaczenie warto�ci segmentu pierwszej figury od �rodka
	for (int i = Center.y; i < card.corner2.y; ++i){
		for (int j = Center.x; j < card.corner2.x; ++j){
			if (out(j, i).I() != 0){
				value = out(j, i).I();
				goto endloop;
			}
		}
	}
endloop:

	// Obliczenie pola oraz obwodu
	for (int i = card.corner1.y; i < card.corner2.y; ++i){
		for (int j = card.corner1.x; j < card.corner2.x; ++j){
			if (out(j, i).I() == value){
				obwod += circuitCalc(out, j, i);
				pole++;
			}
		}
	}
	// Wyznaczenie ratio jako stosunek obwodu do pola
	card.ratio = double(obwod) / double(pole);
}

// Funkcja znajduj�ce rogi kart
void findEdges(Image1CH src, Card cards[]){
	int iter = 0; // Iterator
	int margin_size = 4; // Margines ze wzgl�du na zminimalizowanie ryzyka b�edu na brzegach obrazu

	// Wyznaczenie karty o rogu najblizej lewej kraw�dzi
	for (int i = margin_size; i < src.width() - margin_size; ++i){ // Iteracja po obrazie od lewego g�rnego rogu
		for (int j = margin_size; j < src.height() - margin_size; ++j){
			if (src(i, j).I() == 1){ // Gdy znajdzie piksel karty wpisz do danej karty jego wsp�rz�dne
				cards[0].corner1.x = i;
				cards[0].corner1.y = j;
				goto endloop1; // Skocz do linii endloop1
			}
		}
	}
endloop1:
	// Wyznacznenie karty o rogu najbli�ej prawej kraw�dzi
	for (int i = src.width() - margin_size; i > margin_size; --i){
		for (int j = src.height() - margin_size; j > margin_size; --j){
			if (src(i, j).I() == 1){
				cards[1].corner1.x = i;
				cards[1].corner1.y = j;
				goto endloop2;
			}
		}
	}
endloop2:
	// Wyr�wnanie dolnej granicy
	Compare(cards[1].corner1.y, cards[0].corner1.y, 0);

	// Wyznaczenie karty o rogu najwy�ej g�rnej kraw�dzi
	for (int i = margin_size; i < src.height() - margin_size; ++i){
		for (int j = margin_size; j < src.width() - margin_size; ++j){
			if (src(j, i).I() == 1){
				cards[2].corner1.x = j;
				cards[2].corner1.y = i;
				goto endloop3;
			}
		}
	}
endloop3:
	// Wyr�wnanie lewej granicy
	Compare(cards[0].corner1.x, cards[2].corner1.x, 1);

	// Wyznaczam wsp�rz�dne punktu kt�ry b�dzie wskazywa� na �rodek pomi�dzy kartami znaj�c u�o�enie kart oraz wprowadzaj�c �wiadomy offset
	Point2D Center(((cards[0].corner1.x + cards[1].corner1.x) / 2) - 10, ((cards[2].corner1.y + cards[0].corner1.y) / 2) + 10);

	// Wyznaczam najbli�szy r�g karty w kierunku po�udniowo wschodnim od �rodka kart
	for (int i = Center.x; i < src.width() - margin_size; ++i){
		for (int j = Center.y; j < src.height() - margin_size; ++j){
			if (src(i, j).I() == 1){
				cards[1].corner2.x = i;
				cards[1].corner2.y = j;
				goto endloop4;
			}
		}
	}
endloop4:

	// Zmienna offset przechowuje offset eliminuj�cy nier�wno�ci kart
	int offset = 45;
	// Wyznaczam warto�ci r�nicy x i y od �rodka kart do ich rog�w
	int deltax = std::abs(cards[1].corner2.x - Center.x) + offset;
	int deltay = std::abs(cards[1].corner2.y - Center.y) + offset;
	// Wyznaczam najbli�sze rogi od �rodka ka�dej z kart, wygl�dem przypomina kod sphagetti lecz znagi przy deltach si� zmieniaj� zale�nie od karty
	cards[0].corner2.x = Center.x - deltax;
	cards[0].corner2.y = Center.y + deltay;
	cards[2].corner2.x = Center.x - deltax;
	cards[2].corner2.y = Center.y - deltay;
	cards[3].corner2.x = Center.x + deltax;
	cards[3].corner2.y = Center.y - deltay;
	// Wyznaczam wsp�rz�dne karty 3 na podstawie wsp�rz�dnych innych kart
	cards[3].corner1.x = cards[1].corner1.x - offset;
	cards[3].corner1.y = cards[2].corner1.y + offset;
	// Wyr�wnuj� granic� kart w zale�no�ci czy chcemy granic� dalej czy bli�ej
	Compare(cards[0].corner2.x, cards[2].corner2.x, 0);
	Compare(cards[2].corner1.y, cards[3].corner1.y, 1);
	Compare(cards[1].corner1.x, cards[3].corner1.x, 0);
	Compare(cards[3].corner1.y, cards[2].corner1.y, 1);
	Compare(cards[1].corner2.x, cards[3].corner2.x, 1);
}

// Funkcja obniz�j�ca jasno�� zdj�cia w okre�lonym przedziale (0.1-0.4)
void MakeDarker(Image1CH &src){
	double value = 0;
	for (int i = 0; i < src.width(); ++i){ //Iteracja po obrazie
		for (int j = 0; j < src.height(); ++j){
			if (src(i, j).I()>0.10&&src(i, j).I() < 0.4){ // Je�li intensywno�� piksela zawiera si� w przedziale (0.1-0.4)
				value = src(i, j).I() - 0.15; // Zmniejsz intensywno�� o 0.15
				if (value < 0)value = 0; // Warunek aby nie wpisa� warto�ci ujemnej intensywno�ci
				src(i, j).I() = value;
			}
		}
	}
}

// Funcja do wy�wietlania kart na obrazie
void cardView(Image1CH src, Image1CH& out, Card card){
	if (card.corner1.x > card.corner2.x){ // Je�li warto�� x wi�ksza w 1 rogu od 2, podmie� 'x'
		int temp = card.corner1.x; // Przechowuj� tymczasowo x
		card.corner1.x = card.corner2.x;
		card.corner2.x = temp;
	}
	if (card.corner1.y > card.corner2.y){ // Je�li warto�� y wi�ksza w 1 rogu od 2, podmie� 'y'
		int temp = card.corner1.y;// Przechowuj� tymczasowo y
		card.corner1.y = card.corner2.y;
		card.corner2.y = temp;
	}
	for (int i = card.corner1.x; i < card.corner2.x; ++i){
		for (int j = card.corner1.y; j < card.corner2.y; j++){
			out(i, j).I() = src(i, j).I(); // Przepisz do obrazu wyjsciowego intensywnosc w granicach karty
		}
	}
}

// Procedura sprawdzaj�ca czy zdj�cie ma du�y gradient
int GradientCheck(Image1CH src){

	Image1CH temp(src.width(), src.height()); // temp jest pomocniczym obrazem klasy Image1CH
	Sobel(src, temp); //Wykonujemy filtracje Sobela / Liczymy gradient

	int all_pixels = src.width()*src.height(); // Liczba pikseli w obrazie
	int counter = 0; // Licznik
	for (int i = 0; i < temp.width(); ++i){
		for (int j = 0; j < temp.height(); ++j){
			if (temp(i, j).I() == 1)counter++; // Je�li piksel jest jedyn� zwi�ksz licznik
		}
	}
	double ratio = counter / double(all_pixels); // Ratio przechowuje stosunek '1' do wszystkich pikseli
	// Zale�nie od ratio zwr�� 0,1 lub 2
	if (ratio < 0.0001)
	{
		return 1;
	}
	else
	{
		if (ratio > 0.1)
		{
			return 2;
		}
		else
		{
			return 0;
		}
	}
}
// Funkcja kopiuj�ca obrazy, przepisuj�ca z src wszystko do out
void CopyImages(Image1CH src, Image1CH& out){
	if (src.width() == out.width() && src.height() == out.height()){
		for (int i = 0; i < src.width(); ++i){
			for (int j = 0; j < src.height(); ++j){
				out(i, j).I() = src(i, j).I();
			}
		}
	}
	else
	{
		std::cout << "Rozmiary sie nie zagdzaja" << std::endl;
	}
}
// Funkcja licz�ca karty jako warunek projektu
void CalculateCards(Card cards[]){
	std::cout << "Karty ktore daja reszte dzielenia przez 3 rowna 1 to: ";
	for (int i = 0; i < 4; ++i){ // Przej�cie po wyszstkich kartach
		if (cards[i].value % 3 == 1){ // Je�li reszta z dzielenia r�wna 1 wypisz kart�
			std::cout << cards[i].value << " " << cards[i].kolor << "; ";
		}
	}
	std::cout << std::endl;
}

// Funkcja wbieraj�ca znak karty
void ChooseColour(Card cards[]){
	int pos[2]; // Przechowuje pozycje w tablicy
	int iter = 0;
	for (int i = 0; i < 4; ++i){ // Funkcja znajduj�ca czerwone karty
		if (cards[i].barwa == "Czerwony"){
			pos[iter] = i; // Wpisanie do tablicy pozycji
			iter++; // Inkrementacja iteratora aby wpisa� kolejny do innego pola
		}
	}
	if (cards[pos[0]].ratio < cards[pos[1]].ratio){ // Zale�nie od ratio wyznacz znak
		cards[pos[0]].kolor = "Karo";
		cards[pos[1]].kolor = "Kier";
	}
	else
	{
		cards[pos[0]].kolor = "Kier";
		cards[pos[1]].kolor = "Karo";
	}
	iter = 0; // Zerowanie iteratora
	for (int i = 0; i < 4; ++i){ // Funkcja znajduj�ca czarne karty
		if (cards[i].barwa == "Czarny"){
			pos[iter] = i; // Wpisanie do tablicy pozycji
			iter++; // Inkrementacja iteratora aby wpisa� kolejny do innego pola
		}
	}
	if (cards[pos[0]].ratio < cards[pos[1]].ratio){ // Zale�nie od ratio wyznacz znak
		cards[pos[0]].kolor = "Trefl";
		cards[pos[1]].kolor = "Pik";
	}
	else
	{
		cards[pos[0]].kolor = "Pik";
		cards[pos[1]].kolor = "Trefl";
	}
}
int main()
{
	std::cout << "Aplikacja do rozrozniania kart " << std::endl << std::endl;
	std::cout << "Wybierz zdjecie do analizy: " << std::endl;
	std::cout << "1. Zdjecie referencyjne / idealne" << std::endl;
	std::cout << "2. Zdjecie gradientowe" << std::endl;
	std::cout << "3. Zdjecie zaszumione" << std::endl;
	std::cout << "4. Zdjecie rozmazane" << std::endl << std::endl;
	std::cout << "Podaj numer zdjecia:" << std::endl;
	Image3CH ColourImage(1928, 1448); // Tworz� obiekt klasy Image3CH przechowuj�cy barwny obraz
	int N = 0; // Inicjalizacja zmiennej zdj�cia
	std::cin >> N; // Wczytanie warto�ci N
	while (N > 4 || N < 1) // Warunek sprawdzaj�cy poprawno�� podania liczby nieparzystej
	{
		std::cout << "Podano nieprawidlowa liczbe. Podaj liczb� zdjecia" << std::endl;
		std::cin.clear(); // Usuniecie flagi b��du 
		std::cin.sync(); // Wyczyszczenie bufora
		std::cin >> N;
	}
	switch (N){
	case 1:
		ColourImage.LoadImage("img\\ideal.png", LPL_LOAD_ORIGINAL); // Za�adowanie obrazu 
		break;
	case 2:
		ColourImage.LoadImage("img\\gradient.png", LPL_LOAD_ORIGINAL); // Za�adowanie obrazu 
		break;
	case 3:
		ColourImage.LoadImage("img\\noised.png", LPL_LOAD_ORIGINAL); // Za�adowanie obrazu 
		break;
	case 4:
		ColourImage.LoadImage("img\\blurred.png", LPL_LOAD_ORIGINAL); // Za�adowanie obrazu 
		break;
	default:
		std::cout << "Podaj numer istniejacego zdjecia " << std::endl;
		break;
	}
	ColourImage.ShowImage("Obraz wejsciowy");
	Image1CH GrayscaleImage(1928, 1448); // Tworz� obiekt klasy Image1CH przechowuj�cy szary obraz
	rgbTogray(ColourImage, GrayscaleImage); // Konwertowanie barwnego obrazu do skali szaro�ci
	// Sprawdzenie czy zdjecie jest zaklocone
	int type = GradientCheck(GrayscaleImage);
	if (type == 1){ // Jesli type == 1 to oznacza zdjecie rozmazane, nalezy zmniejszyc jasnosc
		MakeDarker(GrayscaleImage);
	}
	if (type == 2){ // Jesli type == 2 to oznacza zdjecie zaszumione, nalezy zastosowac filtr medianowy
		Image1CH temp_GrayscaleImage(1928, 1448);
		CopyImages(GrayscaleImage, temp_GrayscaleImage);
		Mediana(temp_GrayscaleImage, GrayscaleImage);
	}
	Image1CH BinaryImage(1928, 1448); // Tworz� obiekt klasy Image1CH przechowuj�cy zbinaryzowany obraz
	Binary(GrayscaleImage, BinaryImage); // Binaryzacja
	BinaryImage.ShowImage("Zbinaryzowany");
	Card Cards[4]; // Tworz� tablice kart
	Image1CH GrayscaleImage2(1928, 1448); // Tworz� obiekt klasy Image1CH do segmentacji
	Image1CH GrayscaleImage3(1928, 1448); // Tworz� obiekt klasy Image1CH do wyswietlenia kart
	findEdges(BinaryImage, Cards); // Funkcja zajdujaca rogi zdj��
	for (int i = 0; i < 4; ++i){ // Iteracja po kartach
		cardView(BinaryImage, GrayscaleImage3, Cards[i]);
	}
	GrayscaleImage3.ShowImage("Karty");

	for (int i = 0; i < 4; ++i){ // Iteracja po kartach
		Segmentation(ColourImage, BinaryImage, GrayscaleImage2, Cards[i]); // Wykonanie segmentacji dla kazdej z kart
	}
	GrayscaleImage2.ShowImage("Po segmentacji znakow");
	ChooseColour(Cards); // Funkcja wyznaczajaca znak/kolor karty
	for (int i = 0; i < 4; ++i){ // Iteracja po kartach
		Cards[i].Print(); // Wypisywanie danych 
	}
	std::cout << std::endl;
	CalculateCards(Cards); // Wyznaczenie kart podzielnych przez 3 z reszt� 1
	system("pause");
	return 0;
}