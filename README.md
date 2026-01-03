# NOTAS

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
