import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft

sample_freq = 1000
step = 1 /sample_freq

y = np.genfromtxt('C:/Users/alanb/OneDrive/Documentos/Arduino/Proyectos_ESP32/I2S_SD_esp32/Analisis-medicion/data_3.txt')
y = y.tolist()

# x = np.arange(0,len(y)*step, step)

# for i in range(len(x)):
#     x[i] = x[i]*10

end_range = len(y)

x = np.arange(0,end_range,1)

# Calcula la FFT

print(len(y))
print(len(x))

plt.figure(figsize=(10,5))
plt.plot(x,y)
plt.title('Señal de AD9833', fontsize=15)
plt.xlabel('Muestras', fontsize= 11)
plt.ylabel('Amplitud', fontsize= 11)

# plt.xticks(fontsize= 9)
# plt.yticks(fontsize= 9)

plt.grid()
plt.show()

# yf = fft(y)
# xf = np.linspace(0.0, sample_freq/2, len(y)//2)
# # Grafica la FFT
# plt.figure(figsize=(10,5))
# plt.plot(xf, 2.0/len(y) * np.abs(yf[0:len(y)//2]))
# plt.title('FFT de la señal de salida del mezclador', fontsize=15)
# plt.xlabel('Frecuencia (Hz)', fontsize= 11)
# plt.ylabel('Amplitud', fontsize= 11)

# plt.xticks(fontsize= 9)
# plt.yticks(fontsize= 9)

# plt.grid()
# plt.show()