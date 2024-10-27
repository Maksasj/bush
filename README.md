## Antra užduotis

Šios užduoties tikslas – parodyti tvirtą supratimą apie darbą su Unix procesais naudojant C programavimo kalbą. Programa turi sąveikauti su procesais, priimti ir apdoroti komandas bei atitikti toliau nurodytus reikalavimus:

[x] Parašykite savo kodą C programavimo kalba.
[x] Į kodą įtraukite pakankamai komentarų, kad būtų aiškiau ir paaiškintumėte savo logiką bei požiūrį.
[x] Įsitikinkite, kad kodas gali veikti universiteto Linux kompiuteryje.
[x] Sukurkite funkcinį Makefile savo programai ir įsitikinkite, kad kodas gali būti sukompiliuotas naudojant jį.
[x] Paleidę programą, parodykite vartotojui kviečiantį pranešimą.
[x] Programa turi priimti ir apdoroti vartotojo komandas.
[x] Programa baigiama vienu iš šių būdų:
  [x] Gavęs EOF (failo pabaiga).
  [x] Gavęs iš vartotojo komandą "exit".
[x] Nenaudokite `system(3)` funkcijos komandoms vykdyti.
[x] Naudojant tik standartines Unix API funkcijas, tokias kaip `fork(2)`, `execv(2)`, `exit(2)`, ...
[x] Procesų valdymui nenaudoti išorinių bibliotekų.
[x] Pasirinkite vieną iš šių reikalavimų:
  [x] konvejeriai

     Realizuokite konvejerių palaikymą, kad vieno proceso išvestis būtų naudojama kaip kito proceso įvestis. Pavyzdžiui, vartotojai turėtų turėti galimybę vykdyti tokias komandas kaip `command1 | command2` perduoti `command1` išvestį kaip įvestį `command2`.

  [-] foninis apdorojimas

     Realizuokite apdorojimą fone pagal UNIX `/bin/sh` apvalkalą (shell), realizuodami komandas `bg`, `fg`, `kill` valdymo operatorių `&`.

  [-] srautų nukreipimas

     Realizuokite srautų nukreipimo funkcionalumą pagal UNIX `/bin/sh` apvalkalą (shell), jame nukreipimai atliekami naudojant operatorius `>`, `>>`, `<`, `n>`, ...
[x] Pastaba: nereikia realizuoti išorinių programų, tokių kaip `ls(1)`, `sort(1)` ar `grep(1)`. Vietoj to turėtumėte iškviesti šias programas naudodami tokias funkcijas kaip `execv(2)`.
