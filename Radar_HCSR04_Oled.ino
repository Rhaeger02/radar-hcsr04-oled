#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// ================= CONFIGURACIÓN OLED =================
#define ANCHO_PANTALLA 128
#define ALTO_PANTALLA 64
#define CENTRO_X 64      // Centro horizontal de la pantalla
#define CENTRO_Y 32      // Centro vertical del radar
#define RADIO_MAX 30     // Radio máximo del radar en píxeles

// ================= CONFIGURACIÓN PINES =================
// Pines del sensor HC-SR04
const int trigPin = 9;
const int echoPin = 10;

// Pin del servo
const int pinServo = 6;

// ================= VARIABLES GLOBALES =================
Adafruit_SSD1306 pantalla(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);
Servo radarServo;

// Historial de puntos detectados (para dibujar rastro)
const int MAX_HISTORIAL = 20;
float historialDistancias[MAX_HISTORIAL];
int historialAngulos[MAX_HISTORIAL];
int indiceHistorial = 0;

// ================= FUNCIÓN SETUP =================
void setup() {
  Serial.begin(9600);
  
  // Inicializar OLED
  if(!pantalla.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Error al inicializar OLED"));
    while(true); // Detener si hay error
  }
  
  // Configurar pines del sensor ultrasónico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Inicializar servo
  radarServo.attach(pinServo);
  radarServo.write(90); // Posición inicial (centro)
  
  // Limpiar pantalla
  pantalla.clearDisplay();
  pantalla.setTextSize(1);
  pantalla.setTextColor(SSD1306_WHITE);
  
  // Dibujar interfaz inicial
  dibujarInterfazFija();
  pantalla.display();
  
  // Inicializar historial
  for(int i = 0; i < MAX_HISTORIAL; i++) {
    historialDistancias[i] = 0;
    historialAngulos[i] = 0;
  }
  
  delay(1000); // Pausa inicial
}

// ================= FUNCIÓN LOOP PRINCIPAL =================
void loop() {
  // Barrido de 0 a 180 grados
  for(int angulo = 0; angulo <= 180; angulo += 3) {
    // 1. MOVER SERVO
    radarServo.write(angulo);
    delay(30); // Tiempo para que el servo se estabilice
    
    // 2. MEDIR DISTANCIA
    float distancia = medirDistancia();
    
    // 3. GUARDAR EN HISTORIAL (si es una lectura válida)
    if(distancia > 2 && distancia < 200) {
      guardarEnHistorial(angulo, distancia);
    }
    
    // 4. DIBUJAR EN PANTALLA
    actualizarPantalla(angulo, distancia);
    
    // Pequeña pausa entre mediciones
    delay(20);
  }
  
  // Barrido inverso (180 a 0 grados)
  for(int angulo = 180; angulo >= 0; angulo -= 3) {
    // 1. MOVER SERVO
    radarServo.write(angulo);
    delay(30);
    
    // 2. MEDIR DISTANCIA
    float distancia = medirDistancia();
    
    // 3. GUARDAR EN HISTORIAL
    if(distancia > 2 && distancia < 200) {
      guardarEnHistorial(angulo, distancia);
    }
    
    // 4. DIBUJAR EN PANTALLA
    actualizarPantalla(angulo, distancia);
    
    delay(20);
  }
}

// ================= FUNCIONES AUXILIARES =================

// Función para medir distancia con HC-SR04
float medirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duracion = pulseIn(echoPin, HIGH);
  float distancia = duracion * 0.034 / 2; // Convertir a cm
  
  // Filtrar lecturas erróneas
  if(distancia > 400 || distancia < 2) {
    return 0; // 0 indica fuera de rango
  }
  
  return distancia;
}

// Función para dibujar la interfaz fija del radar
void dibujarInterfazFija() {
  // 1. Dibujar círculos concéntricos (3 líneas)
  for(int r = 10; r <= RADIO_MAX; r += 10) {
    pantalla.drawCircle(CENTRO_X, CENTRO_Y, r, SSD1306_WHITE);
  }
  
  // 2. Dibujar líneas de ángulo (cada 45 grados en semicírculo)
  for(int ang = -90; ang <= 90; ang += 45) {
    float rad = ang * PI / 180.0;
    int x = CENTRO_X + RADIO_MAX * cos(rad);
    int y = CENTRO_Y - RADIO_MAX * sin(rad);
    pantalla.drawLine(CENTRO_X, CENTRO_Y, x, y, SSD1306_WHITE);
  }
  
  // 3. Dibujar título
  pantalla.setCursor(40, 0);
  pantalla.print("RADAR ACTIVO");
  
  // 4. Dibujar línea horizontal (base del semicírculo)
  pantalla.drawLine(CENTRO_X - RADIO_MAX, CENTRO_Y, 
                    CENTRO_X + RADIO_MAX, CENTRO_Y, SSD1306_WHITE);
}

// Función para guardar mediciones en el historial
void guardarEnHistorial(int angulo, float distancia) {
  historialAngulos[indiceHistorial] = angulo;
  historialDistancias[indiceHistorial] = distancia;
  
  indiceHistorial++;
  if(indiceHistorial >= MAX_HISTORIAL) {
    indiceHistorial = 0;
  }
}

// Función para actualizar toda la pantalla
void actualizarPantalla(int anguloActual, float distanciaActual) {
  // 1. Limpiar área dinámica (mantener interfaz fija)
  pantalla.fillRect(0, 12, 128, 52, SSD1306_BLACK);
  
  // 2. Redibujar interfaz fija
  dibujarInterfazFija();
  
  // 3. Dibujar historial de puntos (rastro)
  dibujarHistorial();
  
  // 4. Dibujar línea de barrido actual
  dibujarLineaBarrido(anguloActual);
  
  // 5. Dibujar objeto actual si hay detección
  if(distanciaActual > 2 && distanciaActual < 200) {
    dibujarObjeto(anguloActual, distanciaActual);
  }
  
  // 6. Mostrar información numérica
  mostrarInfo(anguloActual, distanciaActual);
  
  // 7. Actualizar pantalla
  pantalla.display();
}

// Función para dibujar el historial de puntos
void dibujarHistorial() {
  for(int i = 0; i < MAX_HISTORIAL; i++) {
    if(historialDistancias[i] > 0) {
      // Convertir ángulo y distancia a coordenadas
      float anguloRad = (historialAngulos[i] - 90) * PI / 180.0;
      float radio = map(historialDistancias[i], 2, 100, 5, RADIO_MAX);
      radio = constrain(radio, 5, RADIO_MAX);
      
      int x = CENTRO_X + radio * cos(anguloRad);
      int y = CENTRO_Y - radio * sin(anguloRad);
      
      // Dibujar punto (más pequeño para historial)
      pantalla.drawPixel(x, y, SSD1306_WHITE);
    }
  }
}

// Función para dibujar la línea de barrido actual
void dibujarLineaBarrido(int angulo) {
  // Convertir ángulo a coordenadas (0-180 a -90 a 90)
  float anguloRad = (angulo - 90) * PI / 180.0;
  
  int x = CENTRO_X + RADIO_MAX * cos(anguloRad);
  int y = CENTRO_Y - RADIO_MAX * sin(anguloRad);
  
  // Dibujar línea de barrido
  pantalla.drawLine(CENTRO_X, CENTRO_Y, x, y, SSD1306_WHITE);
}

// Función para dibujar un objeto detectado
void dibujarObjeto(int angulo, float distancia) {
  // Convertir ángulo a coordenadas
  float anguloRad = (angulo - 90) * PI / 180.0;
  
  // Mapear distancia a radio en pantalla
  float radio = map(distancia, 2, 100, 5, RADIO_MAX);
  radio = constrain(radio, 5, RADIO_MAX);
  
  // Calcular posición del objeto
  int x = CENTRO_X + radio * cos(anguloRad);
  int y = CENTRO_Y - radio * sin(anguloRad);
  
  // Dibujar objeto (círculo relleno)
  pantalla.fillCircle(x, y, 3, SSD1306_WHITE);
  
  // Dibujar círculo de eco alrededor
  pantalla.drawCircle(x, y, 6, SSD1306_WHITE);
}

// Función para mostrar información numérica
void mostrarInfo(int angulo, float distancia) {
  // Mostrar ángulo actual
  pantalla.setCursor(0, 55);
  pantalla.print("Ang: ");
  pantalla.print(angulo);
  pantalla.print((char)247); // Símbolo de grados
  
  // Mostrar distancia actual
  pantalla.setCursor(60, 55);
  pantalla.print("Dist: ");
  if(distancia > 0 && distancia < 400) {
    pantalla.print(distancia, 0);
    pantalla.print("cm");
  } else {
    pantalla.print("---");
  }
  
  // Mostrar alerta si objeto muy cerca
  if(distancia > 2 && distancia < 20) {
    pantalla.setCursor(40, 45);
    pantalla.print("!PELIGRO!");
  } else if(distancia >= 20 && distancia < 50) {
    pantalla.setCursor(35, 45);
    pantalla.print("OBJETO CERCA");
  }
}

// ================= NOTAS IMPORTANTES =================
/*
CONEXIONES:
OLED -> Arduino:
  GND -> GND
  VCC -> 5V
  SCL -> A5
  SDA -> A4

HC-SR04 -> Arduino:
  VCC -> 5V
  GND -> GND
  Trig -> Pin 9
  Echo -> Pin 10

Servo -> Arduino:
  Rojo (VCC) -> 5V
  Marrón (GND) -> GND
  Naranja (señal) -> Pin 6

MONTAJE:
1. Monta el HC-SR04 sobre el servo (con cinta adhesiva o soporte impreso)
2. Asegúrate de que el servo pueda girar libremente 180°
3. Coloca el conjunto servo-sensor frente al área a escanear

AJUSTES:
1. Si el radar barre muy rápido, aumenta los delays (líneas 67 y 87)
2. Para mayor precisión, reduce el incremento de ángulo (cambia "+= 3" a "+= 2")
3. Ajusta los valores de map() en dibujarObjeto() según tu rango de trabajo

PROBLEMAS COMUNES:
1. Si la pantalla no enciende, verifica la dirección I2C (puede ser 0x3D)
2. Si el servo vibra o no se mueve suave, verifica la alimentación (usar fuente externa si es necesario)
3. Si las lecturas son erráticas, asegura bien las conexiones del HC-SR04
*/