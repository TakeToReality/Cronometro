# Cronometro per arduino
Interfaccia utente con una serie da 6 display 7segmenti comandati in modo parallelo dal microcontrollore, la comunicazione per ogni display avviene in serialmente. Il byte (parola) che rappresenta il numero da visualizzare viene inviato serialmente ad un registro SIPO (Serial Input Parallele Output) che lo riceve e con l'utilizzo di un clock lo mostra sulle sue 8 uscite, così da disporre del byte intero per il display. La scheda si occupa di controllare i pulsanti e calcolare le durate dei lap.
-Adattato per lavorare con scheda esp8266-

Hardware: 6x display 7-segmenti, 6x sipo registri 74ls164, 2x pulsanti

Software: non efficente al massimo ma funziona, diviso a blocchi

Modifiche previste:diverse modalità, luminosità regolabile
