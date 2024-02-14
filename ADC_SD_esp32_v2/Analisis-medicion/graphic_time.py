import numpy as np
import matplotlib.pyplot as plt

# Lee los datos del archivo
data = np.genfromtxt('C:/Users/alanb/OneDrive/Documentos/Arduino/Proyectos_ESP32/ADC_SD_esp32_v2/Analisis-medicion/data_4.txt', delimiter=';')

# Divide los datos en dos listas
y1 = data[:, 0]
y2 = data[:, 1]

# Crea el eje x
x = np.arange(len(y1))

# Grafica los datos
plt.figure(figsize=(10,5))
plt.plot(x, y1, label='ADC1', color='blue')
plt.plot(x, y2, label='ADC2', color='red')
plt.title('Señales de ADC', fontsize=15)
plt.xlabel('Muestras', fontsize= 11)
plt.ylabel('Amplitud', fontsize= 11)
plt.legend()
plt.grid()
plt.show()