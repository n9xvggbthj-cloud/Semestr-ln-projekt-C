# Mini DJ – Správa hudební knihovny a generování mixtapu

Konzolová aplikace v jazyce C pro správu hudební knihovny a automatické generování DJ mixtapu.
Mixtape se skládá podle Camelot systému tónin (1–12) pomocí jednoduchého „random walk“ algoritmu.

## Funkce
- Přidání skladby do knihovny
- Výpis skladeb v tabulce
- Vyhledávání podle názvu nebo interpreta
- Třídění (název, interpret, žánr, Camelot číslo, rok)
- Načtení / uložení knihovny do CSV
- Generování mixtapu o zadané délce
- Volitelná první a poslední skladba v mixtapu

## Datový model
Skladba je uložená jako struktura `Song` (název, interpret, žánr, tónina, rok, délka, BPM).

## Generování mixtapu (stručně)
- Začne se od aktuálního Camelot čísla
- Každý krok zvolí náhodně posun +1 nebo -1 (kruhově 1–12)
- Pokud chybí vhodná skladba, rozšíří hledání do větší vzdálenosti
- Pokud je zadaná cílová skladba (end), ve většině případů se preferuje směr k její tónině

## Autor
Jan Jakoubek
