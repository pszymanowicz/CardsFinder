#include "lib.h"
#include <iostream>
#include <vector>
#include <string>

//LipaLib - Learning Image Processing Autonomic Library

// Strutkura Segments s³u¿y do podmiany wartoœci w obrazie
struct Segments{
	Segments(int n = 0) :swap_from(n), swap_to(n)
	{
	}
	int swap_from; // Zmienna która bêdzie podmieniona
	int swap_to; // Zmienna na któr¹ bêdzie podmiana
};

// Struktura Point2D s³u¿y do przchowania punktu o 2 wspó³rzêdnych x,y
struct Point2D{
	Point2D(int n1 = 0, int n2 = 0) :x(n1), y(n2)
	{
	}
	int x, y;// Wspó³rzêdne punktu
};

// Struktura Card tworzy obiekty przechowuj¹ce karty
struct Card{
	Card(int n = 0, std::string s = ".", double n2 = 0) :corner1(n), corner2(n), kolor(s), barwa(s), ratio(n2), value(n)
	{
	}
	Point2D corner1, corner2; // Pola przechowuj¹ce punkty rogów le¿¹cych na przek¹tnej karty
	std::string kolor; // Pole przechowuj¹ce kolor/znak karty
	std::string barwa; // Pole przechowuj¹ce barwê karty
	double ratio; // Pole przechowuj¹ce stosunek obwodu do pola
	int value; // Pole przechowuj¹ce wartoœæ karty

	void Print(); // Metoda wpisuj¹ca dane karty do konsoli
};

// Metoda wpisuj¹ca dane karty do konsoli
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


// Funkcja realizuj¹ca filtr krawêdziowy Sobela
void Sobel(Image1CH& in, Image1CH& out){
	//Tablice przechowuj¹ce odpowiednie wartoœci maski dla filtru Sobela
	int xTab[9] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
	int yTab[9] = { 1, 1, 1, 0, 0, 0, -1, -1, -1 };

	// Zmienne przechowuj¹ce sumê po x, sumê po y oraz wartoœæ pola
	double sumx = 0, sumy = 0, value;
	int iter = 0; // Iterator
	// Filtracja krawêdziowa
	for (int i = 1; i < in.width() - 1; ++i) //Iteracja po rzêdach
		for (int j = 1; j < in.height() - 1; ++j) //Iteracja po kolumnach
		{
			// Zerowanie sum oraz iteratora dla ka¿dego pixela oddzielnie
			sumx = 0;
			sumy = 0;
			iter = 0;
			for (int k = 0; k < 3; k++)//Iteracja w obrêbie maski
				for (int l = 0; l < 3; ++l){
					sumx += in(i + k - 1, j + l - 1).I()*xTab[iter]; //Dodanie do pola sumx iloczynu maski z obrazem
					sumy += in(i + k - 1, j + l - 1).I()*yTab[iter]; //Dodanie do pola sumy iloczynu maski z obrazem
					iter++; // Inkremenetacja iteratora
				}
			value = std::abs(sumx) + std::abs(sumy); //Wyzaczenie wartoœci pixela na podstawie sumy modu³ów
			if (value > 1)value = 1; // Zabezpieczenie przed zbyt du¿¹ wartoœci¹
			out(i, j).I() = value; // Wpisanie do pixela nowej wartoœci 
		}
}

// Prcedura znajduj¹ca próg binaryzacji
double findTresh(Image1CH src, std::vector<double>& tab){
	for (int i = 0; i < src.width(); ++i){ //Wytworzenie histogramu obrazu
		for (int j = 0; j < src.height(); ++j){
			tab[src(i, j).I() * 255]++;
		}
	}

	double tresh = 0; // Zmienna przechowuj¹ca koñcowy próg
	double temp_tresh = 128; // Próg pocz¹tkowy

	double l_sum = 0, r_sum = 0; // Zmienne przechowuj¹ce sumê po lewo i prawo od progu
	double r_mean = 0, l_mean = 0; // Œrednie na prawo i lewo od progu
	double l_indeks_sum = 0, r_indeks_sum = 0; // Sumy indeksów

	while (temp_tresh != tresh){ //Dopóki tresh jest rózny od temp_tresh
		tresh = temp_tresh; // Przepisz wartoœæ temp_tresh do tresh
		for (int i = 0; i < tresh; ++i){ // Wyznacz sumê indeksów(wyst¹pieñ danej intensywnoœci * intensywnoœæ) oraz sumê po lewo od progu
			l_sum += tab[i];
			l_indeks_sum += tab[i] * i;
		}
		l_mean = l_indeks_sum / l_sum; // Wyznacz œredni¹ na podstawie sumy indeksów i sumy

		for (int i = tresh; i < 256; ++i){ // Wyznacz sumê indeksów oraz sumê po prawo od progu
			r_sum += tab[i];
			r_indeks_sum += tab[i] * i;
		}
		r_mean = r_indeks_sum / r_sum; // Wyznacz œredni¹ na podstawie sumy indeksów i sumy

		temp_tresh = (r_mean + l_mean) / 2; //Wyznacz temp_tresh jako œrednia œredniej prawej i lewej
	}
	tresh = tresh / 255; //Normalizacja progu
	return tresh; // Zwróæ próg
}

// Funkcja binaryzuj¹ca obraz
void Binary(Image1CH src, Image1CH& out){
	double treshold = 0; // Zmienna przechowuj¹ca próg
	std::vector<double> histogram; // Vector doubli przechowuj¹cy histogram obrazu
	histogram.reserve(256); // Zarezerowanie pamiêci na 256 elementów
	for (int i = 0; i < 256; ++i){ // Stworzenie 256 elementów
		histogram.push_back(0);
	}
	treshold = findTresh(src, histogram); // Wyznaczenie progu
	for (int i = 0; i < src.width(); ++i){ //Iteracja po obrazie
		for (int j = 0; j < src.height(); ++j){
			if (src(i, j).I()>treshold){ // Jeœli intensywnoœæ piksela jest wiêksza od progu to wpisz 1, jeœli nie to 0
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
	double e = c[a]; // Element na którego miejsce wskazuje a
	double temp = 0; // Pomocnicza
	while (a < b) // Dopóki a jest mniejsze od b
	{
		while ((a < b) && (c[b] >= e)) b--; // Dopóki b>a oraz element na który wskazuje b>e to dekementacja b
		while ((a < b) && (c[a] < e)) a++; // Dopóki b>a oraz element na który wskazuje a < e to inkrementacja a
		if (a < b) // Jeœli a mniejsze od b to zamieñ miejscami wartoœci na które wskazuj¹ a oraz b
		{
			temp = c[a];
			c[a] = c[b];
			c[b] = temp;
		}
	}
	return a; // Zwróæ a
}
double medianaHoare(double *c, int size)
{
	//algorytm Hoare'a
	int i = 0; // Iterator wskazuj¹cy na pozycjê mediany dzia³aj¹cy od pocz¹tku
	int j = size - 1; // Iterator pomocniczy dzia³aj¹cy od koñca
	int w = j / 2; // Iterator wskazuj¹cy na œrodek przedzia³u
	int k; // Mediana pomocniczna
	while (i != j) // Dopóki i rózne od j
	{
		k = med_temp(c, i, j); // Wpisz do k pozycjê a
		k = k - i + 1;
		if (k >= w) // Jeœli pozycja mediany jest wiêksze lub równe œrodkowi to do j przypisz sume i,k - 1
			j = i + k - 1;
		if (k < w) // Jeœli k jest mniejsze od w to zmniejsz w o k oraz zwieksz i o k
		{
			w -= k;
			i += k;
		}
	}
	return c[i]; // Zwróc element œrodkowy
}
void Mediana(Image1CH src, Image1CH& out)
{
	double Tab[25]; // Inicjalizacja tablicy 
	int iter = 0; // Iterator
	double mediana; // Zmienna przechowowuj¹ca medianê
	for (int i = 2; i < src.width() - 2; ++i){ //Iteracja po obrazie
		for (int j = 2; j < src.height() - 2; ++j){
			iter = 0; // Zerowanie iteratora
			for (int k = 0; k < 5; ++k){ // Iteracja po s¹siedztwie
				for (int l = 0; l < 5; ++l){
					Tab[iter] = src(i + k - 2, j + l - 2).I();  // Wpisanie do tablicy wartoœci intensywnoœci
					iter++; // Inkrementacja iteratora
				}
			}
			mediana = medianaHoare(Tab, 25); // Wykonanie procedury wyznaczenia mediany
			out(i, j).I() = mediana; // Wpisanie mediany do piksela
		}
	}
}

// Prcedura zanjduj¹ca segment
int findSegment(Image1CH out, int x, int y){
	int value = 0; // zmienna przechowuj¹ca wartoœæ segmentu
	for (int i = x - 5; i < x + 5; ++i){ // iteracja o obszarze <-5,5)
		for (int j = y - 5; j < y + 5; ++j){
			if (out(i, j).I()>0)value = int(out(i, j).I()); // jeœli wartoœæ intensywnoœci jest wiêszka od 0 wpisz do value t¹ wartoœæ
		}
	}
	return value; // zwróæ value
}

// Prcedura licz¹ca obwód
int circuitCalc(Image1CH out, int x, int y){
	int suma = 0; // suma przechowuje sumê obwodu
	if (out(x - 1, y).I() == 0)suma++; // Jeœli piksel obok jest 0 oznacza ze jest krawêdzi¹ wliczaj¹c¹ siê do obwodu 
	if (out(x + 1, y).I() == 0)suma++;
	if (out(x, y - 1).I() == 0)suma++;
	if (out(x, y + 1).I() == 0)suma++;
	return suma; // zwróæ sumê
}

// Funkcja porównuj¹ca ze sob¹ dwie liczby
void Compare(int& a, int& b, int type){
	// Jeœli type=1 to znaczy ¿e wyrównujemy do wiêkszej
	if (type == 1){
		if (a > b){ // Jeœli a wiêszke od b wpisz a do b 
			b = a;
		}
		else // Jeœli a mniejsze od b wpisz b do a 
		{
			a = b;
		}
	}
	else{ //Jeœli type=0 to znaczy ¿e wyrównujemy do mniejszej
		if (a > b){ // Jeœli a wiêksze od b wpisz b do a 
			a = b;
		}
		else{ // Jeœli a mniejsze od b wpisz b do a
			b = a;
		}
	}
}

// Funkcja segmentuj¹ca znaki w karcie
void Segmentation(Image3CH clr, Image1CH& src, Image1CH& out, Card& card){
	// Sprawdzenie poprawnosci i kolejnosci rogow w karcie
	if (card.corner1.x > card.corner2.x){ // Jeœli wartoœæ x wiêksza w 1 rogu od 2, podmieñ 'x'
		int temp = card.corner1.x; // Przechowujê tymczasowo x
		card.corner1.x = card.corner2.x;
		card.corner2.x = temp;
	}
	if (card.corner1.y > card.corner2.y){ // Jeœli wartoœæ y wiêksza w 1 rogu od 2, podmieñ 'y'
		int temp = card.corner1.y;// Przechowujê tymczasowo y
		card.corner1.y = card.corner2.y;
		card.corner2.y = temp;
	}

	//Negatyw w danym segmencie
	for (int i = card.corner1.x; i < card.corner2.x; ++i){ //Iteracja po granicach karty
		for (int j = card.corner1.y; j < card.corner2.y; ++j){
			if (src(i, j).I() == 1) // Jeœli piksel jest '1' wpisz 0
			{
				src(i, j).I() = 0;
			}
			else // Jeœli piksel jest '0' wpisz 1
			{
				src(i, j).I() = 1;
			}
		}
	}

	int segment_type = 0, iter = 0; // Zmienne przechowuj¹ce typ segmentu oraz iterator
	double r_channel = 0, b_channel = 0; // Zmienne przechowuj¹ce wartoœæ kana³u czerwonego oraz niebieskiego
	bool check = true; // Zmienna check s³u¿¹ca do wyznaczenia koloru segmentu
	int neighbours_size = 5; // Zmienna s³u¿aca do iteracji po s¹siedztwie piksela o rozmiarze 5
	int margin = (neighbours_size / 2); // Margines do uwzglêdnienia przy warunkach brzegowych
	int counter = 0;
	for (int i = card.corner1.y + margin; i < card.corner2.y - margin; ++i){ // Iteracja po rogach karty
		for (int j = card.corner1.x + margin; j < card.corner2.x - margin; ++j){
			if (src(j, i).I() == 1){ // Jeœli dany piksel jest '1' 
				segment_type = findSegment(out, j, i); // Wywo³anie procedury w celu wyznaczenia segmentu w s¹siedztwie j,i
				if (segment_type == 0){ // Jeœli nie ma segmentu w s¹siedztwie
					iter++; // Inkrementacja iteratora segmentów
					out(j, i).I() = iter; // Wpisz iterator jako nowy segment do piksela
				}
				else{ // Jeœli w s¹siedztwie jest segment
					out(j, i).I() = segment_type; // Wpisz segment do piksela
					counter = 0; //  Wyzeruj licznik
					if (check){ // Jeœli check jest prawd¹ (czyli nie wpisaliœmy jeszcze wartoœci do kana³ów koloru)
						for (int k = 0; k < neighbours_size; ++k){ // Iteracja w s¹siedztwie punktu
							for (int l = 0; l < neighbours_size; ++l){
								if (src(j - k + margin, i - l + margin).I() != 0){ //Jeœli dany piksel jest obiektem inketementuj licznik
									counter++;
								}
							}
						}
						if (counter == 25){ // Jeœli licznik ==25 czyli ca³e s¹siedztwo jest obiektem
							r_channel = clr(j, i).R(); // Wpisz wartoœæ danych kana³ów danego piksela z obrazu kolorowego
							b_channel = clr(j, i).B();
							check = false; // Zmieñ check na false aby nie wykonywaæ tej operacji wiêcej
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
	Segments segment_to_swap; // Zmiennna s³u¿¹ca do wpisania swapa do wektora
	bool exist = false; // Zmienna sprawdzaj¹ca czy dany 'swap' ju¿ istnieje

	for (int i = card.corner1.y + 1; i < card.corner2.y - 1; ++i){ // Iteracja po granicach karty
		for (int j = card.corner1.x + 1; j < card.corner2.x - 1; ++j){
			// Jeœli obecny piksel nie jest zerem, piksel nad nie jest zerem(poniewa¿ iterujemy rzêdami) oraz nie s¹ one równe
			if (out(j, i).I() != 0 && out(j, i - 1).I() != 0 && out(j, i).I() != out(j, i - 1).I()){
				exist = false; // Do exist wpisz false
				for (auto s : swaps){ // Iteracja po wszystkich elementach wektora swaps
					if (s.swap_from == out(j, i - 1).I())exist = true; // Jeœli istnieje ju¿ dany swap wpisz do exist true
				}
				if (!exist){ // Jeœli taki swap nie istenieje tworzymy nowy swap i wpisujemy go do wektora
					segment_to_swap.swap_from = out(j, i - 1).I();
					segment_to_swap.swap_to = out(j, i).I();
					swaps.push_back(segment_to_swap);
				}
			}
		}
	}

	// Przyporz¹dkowanie ka¿demu segmentowi jednej zmiennej
	for (int i = card.corner1.y + 1; i < card.corner2.y - 1; ++i){ //Iteracja po granicach karty
		for (int j = card.corner1.x + 1; j < card.corner2.x - 1; ++j){
			if (out(j, i).I() != 0){ // Jeœli dany piksel nie jest zerem 
				for (auto p : swaps){ // Iteracja po wszystkich elementach wektora swaps
					if (p.swap_from == out(j, i).I())out(j, i).I() = p.swap_to; // Jeœli dana intensywnoœæ piksela zgadza siê z jednym ze swapów przepisz mu odpowiedni¹ wartoœæ
				}
			}
		}
	}
	// Sprawdzenie jakiej barwy s¹ figury na karcie
	if (r_channel - b_channel > 0.15)
	{
		card.barwa = "Czerwony";
	}
	else
	{
		card.barwa = "Czarny";
	}
	// Wartoœæ karty jest to iloœæ wprowadzonych segmentów minus 4 figury na rogach oraz liczba zmian
	card.value = iter - swaps.size() - 4;

	int value = 0, obwod = 0, pole = 0;
	// Wyznaczenie punktu œrodka karty
	Point2D Center((card.corner1.x + card.corner2.x) / 2, (card.corner1.y + card.corner2.y) / 2);

	// Wyznaczenie wartoœci segmentu pierwszej figury od œrodka
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

// Funkcja znajduj¹ce rogi kart
void findEdges(Image1CH src, Card cards[]){
	int iter = 0; // Iterator
	int margin_size = 4; // Margines ze wzglêdu na zminimalizowanie ryzyka b³edu na brzegach obrazu

	// Wyznaczenie karty o rogu najblizej lewej krawêdzi
	for (int i = margin_size; i < src.width() - margin_size; ++i){ // Iteracja po obrazie od lewego górnego rogu
		for (int j = margin_size; j < src.height() - margin_size; ++j){
			if (src(i, j).I() == 1){ // Gdy znajdzie piksel karty wpisz do danej karty jego wspó³rzêdne
				cards[0].corner1.x = i;
				cards[0].corner1.y = j;
				goto endloop1; // Skocz do linii endloop1
			}
		}
	}
endloop1:
	// Wyznacznenie karty o rogu najbli¿ej prawej krawêdzi
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
	// Wyrównanie dolnej granicy
	Compare(cards[1].corner1.y, cards[0].corner1.y, 0);

	// Wyznaczenie karty o rogu najwy¿ej górnej krawêdzi
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
	// Wyrównanie lewej granicy
	Compare(cards[0].corner1.x, cards[2].corner1.x, 1);

	// Wyznaczam wspó³rzêdne punktu który bêdzie wskazywa³ na œrodek pomiêdzy kartami znaj¹c u³o¿enie kart oraz wprowadzaj¹c œwiadomy offset
	Point2D Center(((cards[0].corner1.x + cards[1].corner1.x) / 2) - 10, ((cards[2].corner1.y + cards[0].corner1.y) / 2) + 10);

	// Wyznaczam najbli¿szy róg karty w kierunku po³udniowo wschodnim od œrodka kart
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

	// Zmienna offset przechowuje offset eliminuj¹cy nierównoœci kart
	int offset = 45;
	// Wyznaczam wartoœci ró¿nicy x i y od œrodka kart do ich rogów
	int deltax = std::abs(cards[1].corner2.x - Center.x) + offset;
	int deltay = std::abs(cards[1].corner2.y - Center.y) + offset;
	// Wyznaczam najbli¿sze rogi od œrodka ka¿dej z kart, wygl¹dem przypomina kod sphagetti lecz znagi przy deltach siê zmieniaj¹ zale¿nie od karty
	cards[0].corner2.x = Center.x - deltax;
	cards[0].corner2.y = Center.y + deltay;
	cards[2].corner2.x = Center.x - deltax;
	cards[2].corner2.y = Center.y - deltay;
	cards[3].corner2.x = Center.x + deltax;
	cards[3].corner2.y = Center.y - deltay;
	// Wyznaczam wspó³rzêdne karty 3 na podstawie wspó³rzêdnych innych kart
	cards[3].corner1.x = cards[1].corner1.x - offset;
	cards[3].corner1.y = cards[2].corner1.y + offset;
	// Wyrównujê granicê kart w zale¿noœci czy chcemy granicê dalej czy bli¿ej
	Compare(cards[0].corner2.x, cards[2].corner2.x, 0);
	Compare(cards[2].corner1.y, cards[3].corner1.y, 1);
	Compare(cards[1].corner1.x, cards[3].corner1.x, 0);
	Compare(cards[3].corner1.y, cards[2].corner1.y, 1);
	Compare(cards[1].corner2.x, cards[3].corner2.x, 1);
}

// Funkcja obniz¹j¹ca jasnoœæ zdjêcia w okreœlonym przedziale (0.1-0.4)
void MakeDarker(Image1CH &src){
	double value = 0;
	for (int i = 0; i < src.width(); ++i){ //Iteracja po obrazie
		for (int j = 0; j < src.height(); ++j){
			if (src(i, j).I()>0.10&&src(i, j).I() < 0.4){ // Jeœli intensywnoœæ piksela zawiera siê w przedziale (0.1-0.4)
				value = src(i, j).I() - 0.15; // Zmniejsz intensywnoœæ o 0.15
				if (value < 0)value = 0; // Warunek aby nie wpisaæ wartoœci ujemnej intensywnoœci
				src(i, j).I() = value;
			}
		}
	}
}

// Funcja do wyœwietlania kart na obrazie
void cardView(Image1CH src, Image1CH& out, Card card){
	if (card.corner1.x > card.corner2.x){ // Jeœli wartoœæ x wiêksza w 1 rogu od 2, podmieñ 'x'
		int temp = card.corner1.x; // Przechowujê tymczasowo x
		card.corner1.x = card.corner2.x;
		card.corner2.x = temp;
	}
	if (card.corner1.y > card.corner2.y){ // Jeœli wartoœæ y wiêksza w 1 rogu od 2, podmieñ 'y'
		int temp = card.corner1.y;// Przechowujê tymczasowo y
		card.corner1.y = card.corner2.y;
		card.corner2.y = temp;
	}
	for (int i = card.corner1.x; i < card.corner2.x; ++i){
		for (int j = card.corner1.y; j < card.corner2.y; j++){
			out(i, j).I() = src(i, j).I(); // Przepisz do obrazu wyjsciowego intensywnosc w granicach karty
		}
	}
}

// Procedura sprawdzaj¹ca czy zdjêcie ma du¿y gradient
int GradientCheck(Image1CH src){

	Image1CH temp(src.width(), src.height()); // temp jest pomocniczym obrazem klasy Image1CH
	Sobel(src, temp); //Wykonujemy filtracje Sobela / Liczymy gradient

	int all_pixels = src.width()*src.height(); // Liczba pikseli w obrazie
	int counter = 0; // Licznik
	for (int i = 0; i < temp.width(); ++i){
		for (int j = 0; j < temp.height(); ++j){
			if (temp(i, j).I() == 1)counter++; // Jeœli piksel jest jedyn¹ zwiêksz licznik
		}
	}
	double ratio = counter / double(all_pixels); // Ratio przechowuje stosunek '1' do wszystkich pikseli
	// Zale¿nie od ratio zwróæ 0,1 lub 2
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
// Funkcja kopiuj¹ca obrazy, przepisuj¹ca z src wszystko do out
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
// Funkcja licz¹ca karty jako warunek projektu
void CalculateCards(Card cards[]){
	std::cout << "Karty ktore daja reszte dzielenia przez 3 rowna 1 to: ";
	for (int i = 0; i < 4; ++i){ // Przejœcie po wyszstkich kartach
		if (cards[i].value % 3 == 1){ // Jeœli reszta z dzielenia równa 1 wypisz kartê
			std::cout << cards[i].value << " " << cards[i].kolor << "; ";
		}
	}
	std::cout << std::endl;
}

// Funkcja wbieraj¹ca znak karty
void ChooseColour(Card cards[]){
	int pos[2]; // Przechowuje pozycje w tablicy
	int iter = 0;
	for (int i = 0; i < 4; ++i){ // Funkcja znajduj¹ca czerwone karty
		if (cards[i].barwa == "Czerwony"){
			pos[iter] = i; // Wpisanie do tablicy pozycji
			iter++; // Inkrementacja iteratora aby wpisaæ kolejny do innego pola
		}
	}
	if (cards[pos[0]].ratio < cards[pos[1]].ratio){ // Zale¿nie od ratio wyznacz znak
		cards[pos[0]].kolor = "Karo";
		cards[pos[1]].kolor = "Kier";
	}
	else
	{
		cards[pos[0]].kolor = "Kier";
		cards[pos[1]].kolor = "Karo";
	}
	iter = 0; // Zerowanie iteratora
	for (int i = 0; i < 4; ++i){ // Funkcja znajduj¹ca czarne karty
		if (cards[i].barwa == "Czarny"){
			pos[iter] = i; // Wpisanie do tablicy pozycji
			iter++; // Inkrementacja iteratora aby wpisaæ kolejny do innego pola
		}
	}
	if (cards[pos[0]].ratio < cards[pos[1]].ratio){ // Zale¿nie od ratio wyznacz znak
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
	Image3CH ColourImage(1928, 1448); // Tworzê obiekt klasy Image3CH przechowuj¹cy barwny obraz
	int N = 0; // Inicjalizacja zmiennej zdjêcia
	std::cin >> N; // Wczytanie wartoœci N
	while (N > 4 || N < 1) // Warunek sprawdzaj¹cy poprawnoœæ podania liczby nieparzystej
	{
		std::cout << "Podano nieprawidlowa liczbe. Podaj liczbê zdjecia" << std::endl;
		std::cin.clear(); // Usuniecie flagi b³êdu 
		std::cin.sync(); // Wyczyszczenie bufora
		std::cin >> N;
	}
	switch (N){
	case 1:
		ColourImage.LoadImage("img\\ideal.png", LPL_LOAD_ORIGINAL); // Za³adowanie obrazu 
		break;
	case 2:
		ColourImage.LoadImage("img\\gradient.png", LPL_LOAD_ORIGINAL); // Za³adowanie obrazu 
		break;
	case 3:
		ColourImage.LoadImage("img\\noised.png", LPL_LOAD_ORIGINAL); // Za³adowanie obrazu 
		break;
	case 4:
		ColourImage.LoadImage("img\\blurred.png", LPL_LOAD_ORIGINAL); // Za³adowanie obrazu 
		break;
	default:
		std::cout << "Podaj numer istniejacego zdjecia " << std::endl;
		break;
	}
	ColourImage.ShowImage("Obraz wejsciowy");
	Image1CH GrayscaleImage(1928, 1448); // Tworzê obiekt klasy Image1CH przechowuj¹cy szary obraz
	rgbTogray(ColourImage, GrayscaleImage); // Konwertowanie barwnego obrazu do skali szaroœci
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
	Image1CH BinaryImage(1928, 1448); // Tworzê obiekt klasy Image1CH przechowuj¹cy zbinaryzowany obraz
	Binary(GrayscaleImage, BinaryImage); // Binaryzacja
	BinaryImage.ShowImage("Zbinaryzowany");
	Card Cards[4]; // Tworzê tablice kart
	Image1CH GrayscaleImage2(1928, 1448); // Tworzê obiekt klasy Image1CH do segmentacji
	Image1CH GrayscaleImage3(1928, 1448); // Tworzê obiekt klasy Image1CH do wyswietlenia kart
	findEdges(BinaryImage, Cards); // Funkcja zajdujaca rogi zdjêæ
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
	CalculateCards(Cards); // Wyznaczenie kart podzielnych przez 3 z reszt¹ 1
	system("pause");
	return 0;
}