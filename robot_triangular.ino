#include <MeOrion.h>

// ---------- MOTORES ----------
MeDCMotor motorIzq(M1);
MeDCMotor motorDer(M2);
MeUltrasonicSensor ultra(PORT_3);

int velocidad = 50;

// ---------- ESTADO AUTOMÁTICO ----------
enum Paso {
  AVANZAR,
  GIRAR_DER,
  PAUSAR,
  GIRAR_IZQ,
  RETROCEDER,
  FIN_CICLO
};

Paso pasoActual      = AVANZAR;
unsigned long t_paso = 0;

// Duración de cada paso (ms)
const unsigned long DUR_AVANZAR    = 2000;
const unsigned long DUR_GIRAR_DER  = 800;
const unsigned long DUR_PAUSAR     = 600;
const unsigned long DUR_GIRAR_IZQ  = 800;
const unsigned long DUR_RETROCEDER = 2000;

// ---------- MODOS ----------
enum Modo { AUTOMATICO, MANUAL };
Modo modoActual = MANUAL;

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  parar();
  Serial.println("Sistema iniciado. Modo: MANUAL");
}

// ---------- LOOP ----------
void loop() {
  double distancia = ultra.distanceCm();

  if (Serial.available()) {
    char c = Serial.read();
    Serial.print("Recibido: "); Serial.println(c);

    if (c == 'M') {
      modoActual = MANUAL;
      parar();
      Serial.println("Modo MANUAL");
    }
    else if (c == 'A') {
      modoActual = AUTOMATICO;
      pasoActual = AVANZAR;
      t_paso     = millis();
      Serial.println("Modo AUTOMATICO");
    }
    else if (modoActual == MANUAL) {
      controlManual(c);
    }
  }

  if (modoActual == AUTOMATICO) {
    modoAutomatico(distancia);
  }
}

// ---------- AVANZA AL SIGUIENTE PASO ----------
void siguientePaso() {
  parar();
  pasoActual = (Paso)(pasoActual + 1);
  t_paso = millis();
  Serial.print("Paso: "); Serial.println(pasoActual);
}

// ---------- MODO AUTOMÁTICO ----------
void modoAutomatico(double distancia) {
  unsigned long elapsed = millis() - t_paso;

  // Sensor: parar y retroceder si detecta obstáculo
  if (distancia > 0 && distancia <= 9) {
    parar();
    Serial.println("Obstáculo detectado - parando");
    delay(500);
    retroceder();
    delay(800);
    parar();
    t_paso = millis();
    return;
  }

  switch (pasoActual) {

    case AVANZAR:
      avanzar();
      if (elapsed >= DUR_AVANZAR) siguientePaso();
      break;

    case GIRAR_DER:
      girarDerecha();
      if (elapsed >= DUR_GIRAR_DER) siguientePaso();
      break;

    case PAUSAR:
      parar();
      if (elapsed >= DUR_PAUSAR) siguientePaso();
      break;

    case GIRAR_IZQ:
      girarIzquierda();
      if (elapsed >= DUR_GIRAR_IZQ) siguientePaso();
      break;

    case RETROCEDER:
      retroceder();
      if (elapsed >= DUR_RETROCEDER) siguientePaso();
      break;

    case FIN_CICLO:
      pasoActual = AVANZAR;
      t_paso     = millis();
      Serial.println("-- Ciclo reiniciado --");
      break;
  }
}

// ---------- MODO MANUAL ----------
void controlManual(char c) {
  switch (c) {
    case 'w': avanzar();        Serial.println("Avanzar");    break;
    case 's': retroceder();     Serial.println("Retroceder"); break;
    case 'a': girarIzquierda(); Serial.println("Izquierda");  break;
    case 'd': girarDerecha();   Serial.println("Derecha");    break;
    case 'x': parar();          Serial.println("Parar");      break;
    default:  Serial.println("Comando no reconocido");        break;
  }
}

// ---------- MOVIMIENTO ----------
void avanzar()        { motorIzq.run(-velocidad);   motorDer.run(velocidad);   }
void retroceder()     { motorIzq.run(velocidad);    motorDer.run(-velocidad);  }
void girarDerecha()   { motorIzq.run(-velocidad*3); motorDer.run(-velocidad*3);}
void girarIzquierda() { motorIzq.run(velocidad*3);  motorDer.run(velocidad*3); }
void parar()          { motorIzq.run(0);            motorDer.run(0);           }
