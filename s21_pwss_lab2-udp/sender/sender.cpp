﻿// sender.cpp
#include "sender.h"

int main(int argc, char** argv)
{
	WSADATA wsaData; // zainicjalizowanie struktury WSADATA
	// uruchomienie biblioteki WinSock, jeśli błąd wyświetl komunikat i zakończ działanie
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "Uruchomienie zakończone niepowodzeniem.\n";
		return 1;
	}

	sendFile(); // wywołanie funkcji wysyłającej plik do odbiorcy

	WSACleanup(); // zwolnienie zasobów
	system("pause"); // przeciw autmatycznemu wyłączaniu konsoli
	return 0;
}

void sendFile()
{
	// zainicjalizowanie struktury przechowującej dane połączenia dla odbierającego
	struct sockaddr_in receiverSa;
	int sockfd; // zmienna przechowująca deskryptor socketu

	// wywołanie funkcji tworzącej socket typu UDP, jeśli się nie powiedzie, funkcja
	// zwróci wartość -1 i zostanie wyświetlony error w konsoli
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		std::cerr << "Nieudane utworzenie socketu.\n";
		return;
	}

	receiverSa.sin_family = AF_INET; // typ adresu IP wysyłającego (IPv4)
	receiverSa.sin_port = htons(PORT); // port wysyłającego (pobierany z definicji preprocesora)
	// przypisz do struktury adres odbierającego przy pomocy funkcji inet_pton() która konwertuje
	// zapis w postaci tablicy znaków na sieciową kolejność bajtów
	if (inet_pton(AF_INET, RECV_INET, &(receiverSa.sin_addr)) < 0)
	{
		std::cerr << "Podany adres: " << RECV_INET << " nie jest prawidlowym adresem IPv4.\n";
		return;
	}

	// stwórz plik na podstawie nazwy z receiver.h, jeśli się nie powiedzie, wyświetl błąd
	FILE* fileHandler = fopen(FILE_NAME, "rb");
	if (fileHandler == NULL)
	{
		std::cerr << "Nieudane otwarcie pliku " << FILE_NAME << ".\n";
		return;
	}

	// zmienna przechowująca bufor (pojedynczą ramkę) odbieraną od wysyłającego
	char buffer[FRAME_BUFF];

	// ustaw pozycję kursora na końcu pliku
	fseek(fileHandler, 0, SEEK_END);
	// na podstawie bieżącej pozycji pobierz ilość bajtów pliku
	int fileFullSize = ftell(fileHandler);
	int frames = 0; // ilość wysłanych ramek

	const clock_t begin_time = clock(); // rozpocznij odliczanie czasu

	// pętla iterująca do momentu, kiedy prześle wszystkie bajty (ilość ramek * jej rozmiar)
	while ((frames * FRAME_BUFF) < fileFullSize)
	{
		// przesuń pozycję kursora na podstawie pobieranej ramki (ilość pobranych ramek * jej rozmiar),
		fseek(fileHandler, (frames * FRAME_BUFF), SEEK_SET);
		// odczytaj jedną ramkę pliku i zwróć faktycznie odczytaną ilość bajtów
		size_t singleFrameFileSize = fread(buffer, 1, FRAME_BUFF, fileHandler);

		// uruchomienie funkcji do wysyłania ramki danych do odbierającego, jeśli zwróci -1, błąd
		if (sendto(sockfd, buffer, singleFrameFileSize, 0, (sockaddr*)&receiverSa, sizeof(receiverSa)) < 0)
		{
			std::cerr << "Nieudane wysłanie danych.\n";
			if (closesocket(sockfd) != 0) // zamknięcie socketu
			{
				std::cerr << "Nieudane zamknięcie socketu.\n";
			}
			fclose(fileHandler); // zamknięcie pliku
			return;
		}
		// Sleep(25); opóźnienie, które powoduje odebranie wszystkich ramek u odbiorcy
		frames++; // inkrementuj liczbę wysłanych ramek
	}
	fclose(fileHandler); // zamknięcie pliku

	float elapseTime = float(clock() - begin_time) / CLOCKS_PER_SEC; // oblicz czas wykonywania

	std::cout << "Wyslano " << frames << " ramek, kazda po " << FRAME_BUFF << " bajtow.\n";
	std::cout << "Czas wysylania pliku: " << elapseTime << " sek.\n";
	std::cout << "Calkowity rozmiar pliku: " << fileFullSize << " bajtow.\n";

	if (closesocket(sockfd) != 0) // zamknięcie socketu
	{
		std::cerr << "Nieudane zamkniecie socketu.\n";
	}
}