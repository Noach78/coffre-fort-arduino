#include <Wire.h>
#include "rgb_lcd.h" // Bibliothèque spécifique pour l'écran LCD Grove RGB
#include <Keypad.h>  // Bibliothèque pour le clavier matriciel

rgb_lcd lcd; // Déclaration de l'écran Grove

// Configuration du clavier 4x4
const byte LIGNES = 4; 
const byte COLONNES = 4; 

char keys[LIGNES][COLONNES] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Connexions des broches pour les lignes et colonnes
byte brochesLignes[LIGNES] = {23, 25, 27, 29}; 
byte brochesColonnes[COLONNES] = {31, 33, 35, 37}; 

Keypad keypad = Keypad(makeKeymap(keys), brochesLignes, brochesColonnes, LIGNES, COLONNES);

// notes 
#define NOTE_D4  294
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_AS3 233 
#define NOTE_B3  247 
#define NOTE_C4  262 
#define REST     0  

int melody[] = {
  NOTE_D4, NOTE_D4, NOTE_D5, REST, NOTE_A4, REST, NOTE_GS4, REST, NOTE_G4, REST, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
  NOTE_C4, NOTE_C4, NOTE_D5, REST, NOTE_A4, REST, NOTE_GS4, REST, NOTE_G4, REST, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
  NOTE_B3, NOTE_B3, NOTE_D5, REST, NOTE_A4, REST, NOTE_GS4, REST, NOTE_G4, REST, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4,
  NOTE_AS3, NOTE_AS3, NOTE_D5, REST, NOTE_A4, REST, NOTE_GS4, REST, NOTE_G4, REST, NOTE_F4, NOTE_D4, NOTE_F4, NOTE_G4
};

int durations[] = {
  120, 120, 120, 120, 240, 120, 120, 120, 120, 120, 240, 120, 120, 120,
  120, 120, 120, 120, 240, 120, 120, 120, 120, 120, 240, 120, 120, 120,
  120, 120, 120, 120, 240, 120, 120, 120, 120, 120, 240, 120, 120, 120,
  120, 120, 120, 120, 240, 120, 120, 120, 120, 120, 240, 120, 120, 120
};

int totalNotes = sizeof(melody) / sizeof(melody[0]);
int currentNote = 0;
unsigned long previousMillis = 0;
long currentDuration = 0;

const int pinBouton = 2;
const int pinLedRouge = 4;
const int pinLedOrange = 5;
const int pinLedVerte = 6;
const int pinBuzzer = 8;
const int pinPotentiometre = A0;

// coffre
const int codeSecret[3] = {5, 8, 4}; 
int codeSaisi[3];
int indexSaisie = 0;
int erreurs = 0;

// Remplacement de OPEN par OUVERT pour éviter le conflit avec Keypad.h
enum Etat { LOCKED, CHECKING, OUVERT, ALARM };
Etat etatActuel = LOCKED;

// Variables pour millis()
unsigned long tempsOuverture = 0;
unsigned long tempsAlarme = 0;
unsigned long tempsClignotement = 0;
unsigned long dernierAffichageLCD = 0;
bool etatLedRouge = false;
bool etatLedOrange = false;

// Variables bouton (anti-rebond de 50ms)
unsigned long dernierTempsBouton = 0;
bool dernierEtatBouton = HIGH;
bool etatStableBouton = HIGH;

int lireChiffrePotentiometre() {
  long somme = 0;
  for (int i = 0; i < 10; i++) {
    somme += analogRead(pinPotentiometre);
  }
  int moyenne = somme / 10;
  return map(moyenne, 0, 1023, 0, 9);
}

void mettreAJourLCD(unsigned long tempsActuel, int chiffreActuel) {
  if (tempsActuel - dernierAffichageLCD < 100) return;
  dernierAffichageLCD = tempsActuel;

  lcd.setCursor(0, 0);
  if (etatActuel == LOCKED) {
    for (int i = 0; i < 3; i++) {
      if (i < indexSaisie) {
        lcd.print(codeSaisi[i]);
      } else {
        lcd.print(".");
      }
      if (i < 2) {
        lcd.print("  ");
      }
    }
    lcd.print("       ");
  } else if (etatActuel == CHECKING) {
    lcd.print("VERIFICATION... ");
  } else if (etatActuel == OUVERT) {
    lcd.print("COFFRE OUVERT!  ");
  } else if (etatActuel == ALARM) {
    lcd.print("!! ALARME !!    ");
  }

  lcd.setCursor(0, 1);
  if (etatActuel == LOCKED) {
    lcd.print("Clavier / Pot:");
    lcd.print(chiffreActuel);
    lcd.print(" ");
  } else if (etatActuel == OUVERT) {
    lcd.print("Fermeture auto..");
  } else if (etatActuel == ALARM) {
    lcd.print("Bloque 10s...   ");
  } else {
    lcd.print("                ");
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(pinBouton, INPUT_PULLUP);
  pinMode(pinLedRouge, OUTPUT);
  pinMode(pinLedOrange, OUTPUT);
  pinMode(pinLedVerte, OUTPUT);
  pinMode(pinBuzzer, OUTPUT);

  // Initialisation propre de l'état du bouton pour éviter la fausse saisie au démarrage
  delay(50);
  etatStableBouton = digitalRead(pinBouton);
  dernierEtatBouton = etatStableBouton;

  // Initialisation de l'écran Grove (16 colonnes, 2 lignes)
  lcd.begin(16, 2);
  lcd.setRGB(255, 255, 255);

  Serial.println("Coffre Verrouillé. Saisissez le code.");
  digitalWrite(pinLedRouge, HIGH); 
}

void loop() {
  unsigned long tempsActuel = millis();
  int chiffrePointe = lireChiffrePotentiometre();

  mettreAJourLCD(tempsActuel, chiffrePointe);
  
  bool actionValider = false;
  int chiffreAValider = -1;

  // 1. Lecture du Clavier Matriciel
  char touche = keypad.getKey();
  if (touche) {
    if (touche >= '0' && touche <= '9') {
      chiffreAValider = touche - '0'; // Convertir le caractère en entier
      actionValider = true;
    }
  }

  // 2. Lecture du Bouton poussoir physique (avec le potentiomètre)
  bool lectureBouton = digitalRead(pinBouton);
  if (lectureBouton != dernierEtatBouton) {
    dernierTempsBouton = tempsActuel; 
  }

  if ((tempsActuel - dernierTempsBouton) > 50) {
    if (lectureBouton != etatStableBouton) {
      etatStableBouton = lectureBouton; 
      // On s'assure de valider uniquement sur un front descendant (quand onappuie : HIGH -> LOW)
      // et que l'état précédent stabilisé était bien HIGH (évite les bugs au reset)
      if (etatStableBouton == LOW) {
        chiffreAValider = chiffrePointe;
        actionValider = true; 
      }
    }
  }
  dernierEtatBouton = lectureBouton;

  // Machine à états
  switch (etatActuel) {
    case LOCKED:
      digitalWrite(pinLedRouge, HIGH);
      digitalWrite(pinLedVerte, LOW);
      digitalWrite(pinLedOrange, LOW);
      lcd.setRGB(255, 255, 255);
      
      if (actionValider) {
        tone(pinBuzzer, 1000, 100); 
        codeSaisi[indexSaisie] = chiffreAValider;
        
        Serial.print("Chiffre saisi : ");
        Serial.println(codeSaisi[indexSaisie]);
        indexSaisie++;
      }
      
      // Si on a saisi les 3 chiffres, on attend 800 ms avant de passer à la vérification
      if (indexSaisie >= 3) {
        static unsigned long tempsAttenteFinSaisie = 0;
        if (tempsAttenteFinSaisie == 0) {
          tempsAttenteFinSaisie = tempsActuel; 
        }
        if (tempsActuel - tempsAttenteFinSaisie >= 800) {
          tempsAttenteFinSaisie = 0; 
          etatActuel = CHECKING; 
        }
      }
      break;

    case CHECKING: { 
      bool codeBon = true;
      for (int i = 0; i < 3; i++) {
        if (codeSaisi[i] != codeSecret[i]) codeBon = false;
      }
      
      if (codeBon) {
        Serial.println("Code BON ! Ouverture...");
        erreurs = 0;
        tempsOuverture = tempsActuel;
        etatActuel = OUVERT;
      } else {
        Serial.println("Code FAUX !");
        erreurs++;
        digitalWrite(pinLedOrange, HIGH);
        tone(pinBuzzer, 200, 300); 
        
        if (erreurs >= 3) {
          Serial.println("ALARME DECLENCHEE !");
          tempsAlarme = tempsActuel;
          etatActuel = ALARM;
        } else {
          indexSaisie = 0;
          etatActuel = LOCKED;
        }
      }
      break;
    } 

    case OUVERT:
      digitalWrite(pinLedRouge, LOW);
      digitalWrite(pinLedVerte, HIGH); 
      lcd.setRGB(0, 255, 0);
      
      if (tempsActuel - previousMillis >= currentDuration) {
        previousMillis = tempsActuel;

        int note = melody[currentNote];
        currentDuration = durations[currentNote];

        if (note == REST) {
          noTone(pinBuzzer);
        } else {
          tone(pinBuzzer, note, currentDuration * 0.9);
        }

        currentNote++;
        if (currentNote >= totalNotes) {
          currentNote = 0; 
        }
      }

      if (tempsActuel - tempsOuverture >= 5000) {
        Serial.println("Fermeture automatique.");
        noTone(pinBuzzer); 
        currentNote = 0;   
        indexSaisie = 0;
        etatActuel = LOCKED;
      }
      break;

    case ALARM:
      digitalWrite(pinLedOrange, LOW);
      digitalWrite(pinLedVerte, LOW);
      lcd.setRGB(255, 0, 0);
      
      if (tempsActuel - tempsClignotement >= 200) {
        tempsClignotement = tempsActuel;
        etatLedRouge = !etatLedRouge;
        etatLedOrange = !etatLedOrange;
        digitalWrite(pinLedRouge, etatLedRouge);
        digitalWrite(pinLedOrange, etatLedOrange);
        if (etatLedRouge) tone(pinBuzzer, 1500, 100);
      }
      
      if (tempsActuel - tempsAlarme >= 10000) {
        Serial.println("Fin de l'alarme. Réinitialisation.");
        erreurs = 0;
        indexSaisie = 0;
        digitalWrite(pinLedRouge, HIGH);
        etatActuel = LOCKED;
      }
      break;
  }
}