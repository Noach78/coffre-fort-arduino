# Coffre-fort électronique à code

Ce projet Arduino simule un coffre-fort qui s'ouvre après la saisie d'un code à trois chiffres. Il s'adresse à toute personne qui souhaite découvrir les entrées/sorties Arduino, la lecture d'un clavier et d'un potentiomètre, ainsi que la gestion d'états et de temporisations.

## Fonctionnalités

- Saisie des chiffres avec le clavier matriciel **ou** avec le potentiomètre et le bouton.
- Affichage des chiffres et des messages sur l'écran LCD, et des événements dans le Moniteur Série.
- Code correct : LED verte, mélodie au buzzer, puis reverrouillage automatique après **5 secondes**.
- Code incorrect : signal sonore et LED rouge.
- Après **3 codes incorrects**, alarme sonore et clignotement de la LED rouge pendant **10 secondes**. Les saisies ne sont pas traitées pendant l'alarme.
- Anti-rebond logiciel de **50 ms** pour le bouton.

Le code actuellement configuré est `584`. Il est défini dans `coffre.ino` et peut être modifié dans le tableau `codeSecret`.

## Matériel

- 1 Arduino **Mega 2560** (recommandé : les broches du clavier utilisées dépassent celles d'un Uno)
- 1 clavier matriciel 4 × 4
- 1 potentiomètre
- 1 bouton poussoir
- 1 écran LCD Grove RGB, 16 × 2
- 3 LED : rouge, bleu et verte
- 3 résistances adaptées aux LED (par exemple 220 à 330 Ω)
- 1 buzzer piézoélectrique
- Des fils de connexion et une alimentation USB

## Câblage

Le dépôt ne contient pas de schéma image (`wiring.png`). Le tableau ci-dessous reprend les broches déclarées dans le sketch `coffre.ino`.

| Composant       | Connexion Arduino                                                                                                                          |
| --------------- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| Bouton poussoir | D2 et GND. L'entrée utilise `INPUT_PULLUP` : le bouton relie donc D2 à GND lorsqu'on appuie.                                               |
| LED rouge       | D4, avec une résistance en série ; cathode à GND.                                                                                          |
| LED rouge       | D5, avec une résistance en série ; cathode à GND.                                                                                          |
| LED verte       | D6, avec une résistance en série ; cathode à GND.                                                                                          |
| Buzzer          | Signal sur D8, autre borne sur GND.                                                                                                        |
| Potentiomètre   | Broche centrale sur A0 ; broches extérieures sur 5 V et GND.                                                                               |
| Clavier 4 × 4   | Lignes 1 à 4 sur D23, D25, D27, D29 ; colonnes 1 à 4 sur D31, D33, D35, D37. Vérifiez l'ordre des broches avec les indications du clavier. |
| LCD Grove RGB   | SDA sur SDA et SCL sur SCL (Mega 2560 : D20 et D21) ; alimentation et masse selon le module.                                               |

Reliez les masses des composants à la masse Arduino. Vérifiez les inscriptions de votre écran et de votre clavier avant le branchement : l'ordre physique des connecteurs peut varier d'un modèle à l'autre.

## Installation et utilisation

1. Ouvrez `coffre.ino` dans l'IDE Arduino et sélectionnez une carte Arduino Mega 2560 ainsi que son port.
2. Installez les bibliothèques Arduino **Keypad** et **Grove - LCD RGB Backlight** (fournissant `Keypad.h` et `rgb_lcd.h`). `Wire` est fourni avec l'environnement Arduino.
3. Vérifiez le câblage, puis compilez et téléversez le sketch.
4. Ouvrez le Moniteur Série à **9600 bauds**.
5. Entrez trois chiffres avec les touches numériques du clavier. À défaut, tournez le potentiomètre pour sélectionner un chiffre de 0 à 9, puis appuyez sur le bouton pour le valider. Après le troisième chiffre, le programme attend **800 ms** avant de vérifier le code.

L'écran affiche les chiffres saisis sans les masquer. Le programme peut aussi accepter des chiffres provenant successivement du clavier et du potentiomètre.

## Fonctionnement et choix techniques

Le programme organise le comportement autour de quatre états :

| État       | Fonctionnement                                                                |
| ---------- | ----------------------------------------------------------------------------- |
| `LOCKED`   | Coffre verrouillé, LED rouge allumée, saisie du code.                         |
| `CHECKING` | Comparaison des trois chiffres avec le code configuré.                        |
| `OUVERT`   | Accès simulé, LED verte et mélodie ; retour au verrouillage après 5 secondes. |
| `ALARM`    | Saisie ignorée, LED rouge clignotante et bips pendant 10 secondes.            |

Les temporisations d'ouverture, d'alarme et de mise à jour des LED s'appuient sur `millis()` plutôt que sur des attentes bloquantes dans la boucle principale. Le bouton est filtré par un anti-rebond logiciel. Le potentiomètre est lu dix fois et les valeurs sont moyennées avant conversion en chiffre.

Le clavier permet de saisir directement des chiffres, tandis que le potentiomètre et le bouton offrent une autre méthode de saisie. Les LED rendent les états visibles sans ajouter de mécanisme de verrouillage ; le LCD et le Moniteur Série facilitent l'utilisation et le diagnostic pendant la démonstration.

## Scénario de démonstration

1. Saisir `5`, `8`, `4` : observer la vérification, l'ouverture simulée, la LED verte et la fermeture automatique.
2. Saisir un code incorrect trois fois : observer les signaux d'erreur, puis l'alarme et le blocage temporaire.
3. Essayer de valider le bouton rapidement : l'anti-rebond évite de compter plusieurs pressions dues aux rebonds mécaniques.

## Bilan et améliorations possibles

Le sketch met en œuvre la saisie par deux interfaces, l'affichage LCD et série, la vérification d'un code, l'ouverture temporisée, le comptage des erreurs et le verrouillage temporaire avec alarme. Le code est fourni pour une simulation par indicateurs lumineux ; il ne commande pas de serrure réelle.

Avant une présentation ou une utilisation sur circuit, il reste à vérifier le câblage réel et l'ordre des broches du clavier, et à ajouter un schéma de câblage au dépôt. Pour renforcer la qualité de la démonstration, on pourrait masquer les chiffres sur l'écran, rendre le signal d'erreur plus nettement visible, puis ajouter et tester un actionneur si une ouverture physique est souhaitée. Même avec ces évolutions, ce montage ne doit pas être considéré comme un système de sécurité réel.
