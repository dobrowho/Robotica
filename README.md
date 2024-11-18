# Tema III - Laborator robotică
## 1.Descriere task
Această temă presupune dezvoltarea unui joc interactiv de reflexe, destinat a doi jucători, care își testează viteza de reacție prin apăsarea rapidă a butonului asociat culorii afișate pe LED-ul RGB alocat fiecărui jucător. Pe parcursul unei runde, jucătorii primesc pe rând o culoare aleatorie (roșu, verde sau albastru). Fiecare trebuie să apese rapid butonul corect pentru a obține puncte. Scorurile sunt actualizate automat și afișate pe un ecran LCD. Jocul se încheie după ce servomotorul efectuează o rotație completă, iar scorurile finale sunt afișate.
Principiul de funcționare
Acest sistem implementează un joc de reflexe pentru doi jucători, utilizând două plăci Arduino (Master și Slave) care comunică între ele prin protocolul SPI. Jocul combină elemente hardware precum butoane, LED-uri și un servomotor, cu o logică software bine definită pentru gestionarea rundelor și punctajului.
Inițializarea Jocului
Jocul începe prin apăsarea unui buton conectat la placa Master. După detectarea unui semnal valid pe pinul butonului, Master-ul solicită fiecărui jucător să își introducă numele folosind monitorul serial. Numele introduse sunt afișate pe ecranul LCD atașat plăcii Master, iar jocul trece în modul activ.
Master-ul inițializează cronometrul jocului utilizând funcția millis() și setează durata totală a unei runde. În același timp, servomotorul conectat la placa Master începe să indice progresul jocului, rotindu-se gradual de la 0 la 180 de grade în funcție de timpul scurs.
Desfășurarea Rundelor
Fiecare rundă este alternată între cei doi jucători. La începutul unei runde, Master-ul alege o culoare aleatorie (roșu, verde sau albastru) și o transmite către Slave prin SPI. Slave-ul aprinde LED-ul corespunzător culorii selectate, fie pentru jucătorul 1, fie pentru jucătorul 2, în funcție de a cui este rândul.
Jucătorul activ trebuie să apese un buton care să corespundă culorii afișate. Slave-ul monitorizează răspunsul butonului și măsoară timpul de reacție al jucătorului. Dacă butonul corect este apăsat, Slave-ul calculează punctajul pe baza timpului de răspuns:
Timp rapid: 5 puncte.
Timp moderat: 3 puncte.
Timp lent: 1 punct.
Răspuns incorect sau întârziat: 0 puncte.
Punctajul calculat este transmis înapoi la Master prin SPI. Master-ul actualizează scorul jucătorului și afișează scorurile curente pe ecranul LCD.
## 2.Componente utilizate
- 2x Arduino UNO (ATmega328P microcontroller)
- 2x LED RGB (pentru a primi culorile)
- 6x LED (2x rosu; 2x verde; 2x albastru)
- 7x Butoane (1x START; 6x pentru selectare culori)
- 22x Rezistoare (10x 330 ohm, 12x 220 ohm)
- 2x Breadbord
- 1x Display LCD
- 1x Potentiometer
- 1x Servomotor
- 45x Jumpere
## 3.Poze setup fizic 
![WhatsApp Image 2024-11-18 at 14 27 27](https://github.com/user-attachments/assets/363c103e-832a-4ad1-a366-392cd72c5666)
![WhatsApp Image 2024-11-18 at 14 27 26 (1)](https://github.com/user-attachments/assets/bf5078ec-baf6-4913-bd0a-d790de09fd5a)
![WhatsApp Image 2024-11-18 at 14 27 26](https://github.com/user-attachments/assets/ec7ce7be-e3cb-43a7-a684-efbf7410ec09)
## 4.Schemă electrică (TinkerCAD)
![image](https://github.com/user-attachments/assets/b995fb09-812c-4c27-854c-6bafeb88af4c)
## 5.Link funcționalitate montaj fizic
[https://youtu.be/GkZTBrwhnVA](https://youtu.be/9jz-Yl2SB9M)
