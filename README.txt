 Tema 4 EGC View Frustrum CUlling  -  Bagas Timur 333CA
 
Cuprins:
 
1.Cerinta
2.Utilizare
3.Implementare
Testare
Probleme Aparute
Continutul Arhivei
Functionalitati


1.Cerinta

Am avut de implementat algoritmul de frustrum culling pe cateva sute de obiecte,
de implementat lumina de tip spot si un view general al scenei.

2.Utilizare

Se deschide fisierul .sln din root-ul arhivei si se compileaza si se ruleaza normal
ca orice program din Visual Studio. Programul e controlat in intregime de la tastatura.

Input Tastatura (ca la laboratorul de camere):

WASD - miscari directionale
RF - miscare in inaltime
EQ - rotatii pe Y
ZC - rotatii pe Z
TG - rotatii pe X
TAB -  schimbare view

3.Implementare

IDE: Microsoft Visual Studio
Compilator: Standard C++ Compiler
OS: WIndows
Framework: Frameworkul din laboratorul 3 de OpenGL impreuna cu cateva elemente din laboratorul 6.
Dependinte: Sunt toate in folderul dependente din root

3.1 Algoritmi:

Generare automata a scenei:
- scena formata din ground, cladiri (objects in program), nava
- generarea cladirilor se face dupa urmatorul algoritm:
		- se itereaza cu 2 foruri cu un pas fixat pana la limita din define-ul MAP_SIZE
		- se genereaza geometria pentru fiecare cladire la fiecare iteratie
		- se introduce intr-un vector de structuri ce ne fac mai usoara verificarea varfurilor 
		- mentiune: textura si inaltimea cladirilor este random

Frustrum culling:
- folosind structura dedicata fiecarei cladiri, se verifica daca un varf al cladirii se afla in frustrum, mai exact:
	- dupa define-urile de la inceputul programului care caracterizeaza frustrumul, generam planurile
	- fiecare plan e generat dupa colturile frustrumului
	- pentru fiecare plan aflam ecuatia planului
		- pentru fiecare varf al cladirii:
			- inlocuim in ecuatia planului, iar in functie de semnul rezultatului, ne dam seama daca se afla
			de partea corecta a acestuia (adica in frustrum)
		- daca niciun varf nu se afla pe partea corecta a planului, nu e in frustrum
	- daca pentru toate cele 6 planuri am avut cel putin un varf de obiect valid, deseneaza-l

Schimbarea intre viewuri
- aceasta s-a facut cu ajutorul unei variabile bool si modificarea parametrilor trimisi catre shader in functie de
acea variabila

Pozitionarea navei:
- este mutata in fata camerei, apoi rotita in functie de vectorul forward al camerei si axe
- rotim camera cu arccosinusul din produsul scalar al celor doi vectori

Iluminare (in vertex shader):
- se afla vectorul definit de sursa de lumina si punctul de desenat
- se afla unghiul dintre acesta si normala (forward)
- daca unghiul se incadreaza in limita conului de lumina:
	- creeaza un factor de iluminare in functie de unghiul fata de centru si distanta fata de sursa de iluminare
- altfel trimite 0, ca sa se deseneze totul in umbra

4.Testare

Am testat naveta in destul de multe scenarii, iar cele mai importante (cu probleme) ar fi:
- diferite distante pentru planul near al frustrumului
	- acesta ar putea fi marit din define-uri pentru a scapa de situatia in care unele cladiri nu sunt deloc
	iluminate din anumite pozitii unde ar putea fi din cauza faptului ca nava se afla intr-o cladire
		- am decis s-o las asa pentru ca s-ar pierde din imersion altfel
- la rotatii pe axa Z, pozitia navei nu e statica, n-am reusit sa repar asta
- la rotatiile normale, la unghiuri de 90/270 de grade, nava e pozitionata ciudat, probabil de la erorile de calcul
in virgula mobila
- alegerea dimensiunilor pentru frustrum a necesitat o gramada de testare (am lasat ca modificarile din scena cauzate
de frustrum sa fie vizibile ca sa fie evident ca algoritmul isi face treaba)
- toate celelalte situatii sunt in regula din cate am observat (am verificat cam totul, zic eu, nu le enumar ca sunt prea
multe)

5. Probleme:
- diferenta intre grade si radiani trimise ca parametru la rotatii (mi-a luat jumatate de zi sa-mi dau seama ca trebuia sa
convertesc)
- deoarece am generat singur obiectele si n-am fost in stare sa fac un UV mapper performant, a trebuit sa improvizez si
sa generez coordonatele UV pentru texturi de la dimensiunile corpurilor si la final a iesit chiar bine

6.COntinutul arhivei
- main.cpp -  sursa principala
- lab_camera, lab_geometry, lab_glut, lab_glut_support, lab_mesh_loader, lab_shader_loader, lab_texture_loader
	- frameworkul de lab
- lab_gl_3.sln - fisierul de proiect
- shadere/shader_fragment si shader_vertex - shadere
- resurse/building1 si 2 si 3 si 4 si green, ground, red, ship.BMP - texturile folosite (luate de pe net mai putin cele
cu nume de culori si groundul)
- resurse/SpaceShip.obj - obiectul navei
- dependente/freeglut , glew si glm -  framework de openGL
- README <--- You are here


7. Functionalitati

7.1 Standard:

- generarea a sute/mii de obiecte texturate (controlabil prin MAP_SIZE)
- lumina de tip spot
- control al navei spatiale prin tastatura
- view frustrum culling
- alternate view

7.2 Bonus
- generare random a scenei
- iluminare de tip spot ce tine cont de distanta

7.3 Suplimentare
- cod ultra-controlabil prin variabilele din define-urile de la inceputul fisierului main (va recomand sa va jucati cu ele
daca aveti timp)
- generare automata de UV coordinates