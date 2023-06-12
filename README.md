# Ovládání kotle
Cílem tohoto projektu bylo vytvořit řídicí systém pro vytápění pomocí desky s mikro kontrolérem ESP32 nebo ESP8266. Systém sleduje teplotu pomocí teplotních čidel DS18B20 a upravuje vytápění na základě cílové teploty nastavené uživatelem. Systém obsahuje webové rozhraní, které umožňuje uživatelům sledovat a ovládat vytápění přes webové rozhraní (mobil, tablet, PC, atd.) nebo pomocí vytvořeného termostatu (display a 2 tlačítka). Do budoucna bych chtěl projekt vylepšit o vzdálené ovládání (zatím funguje pouze v domácí síti), větší display, grafy a uvidím, co vymyslím dál. 

Potřebné součástky:
1.	Vývojová deska Arduino ESP8266
2.	Teplotní senzory Dallas DS18B20
3.	2kanálové relé modul
4.	Propojovací kabely – typ Dupont
5.	Montážní krabička – 3D tisk
6.	0.96" OLED displej 
7.	Wago svorky (Místo nepájivého pole)
8.	Rezistory – 4K7, 2x 10k
9.	2x tlačítka 12x12x7.3mm s knoflíkem
