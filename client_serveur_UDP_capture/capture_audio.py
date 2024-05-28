import numpy as np
import pyaudio

########################################
######## Paramètres des micros #########
########################################

RESPEAKER_RATE = 16000
RESPEAKER_CHANNELS = 8
RESPEAKER_WIDTH = 2
RESPEAKER_INDEX = 2  # Modifier avec l'index obtenu précédemment
CHUNK = 256 // 2  # Réduire la taille du tampon pour éviter les débordements
RECORD_SECONDS = 1
dt = 0.1  # pas de temps pour moyenner

# Durée d'affichage en secondes
display_duration = 1.0  # 1 seconde d'affichage
buffer_size = int(RESPEAKER_RATE * display_duration)  # taille du tampon pour 1 seconde
buffers = [np.zeros(buffer_size) for _ in range(6)]

def open_stream():
    p = pyaudio.PyAudio()
    stream = p.open(rate=RESPEAKER_RATE, format=p.get_format_from_width(RESPEAKER_WIDTH), channels=RESPEAKER_CHANNELS, input=True, input_device_index=RESPEAKER_INDEX)
    return p, stream

def close_stream(stream, p):
    stream.stop_stream()
    stream.close()
    p.terminate()

def RMS(signal):
    return np.sqrt(np.mean(signal * signal))

def capture_channel(stream, buffers):
    try:
        data = stream.read(CHUNK, exception_on_overflow=False)
        signals = [np.frombuffer(data, dtype=np.int16)[i+2::8] for i in range(6)] # Le +2 est étonnant mais a l'air nécessaire pour avoir accès aux bons micros dans le bon ordre !
        
        for i in range(6):
            buffers[i] = np.roll(buffers[i], -len(signals[i]))
            buffers[i][-len(signals[i]):] = signals[i]
        
        return buffers
    except IOError as e:
        print(f"Error capturing audio: {e}")
        return buffers

def actualise_les_intensites(buffers):
    intensities = [RMS(buffer) for buffer in buffers]
    return intensities

def actualise_les_dB(buffers):
    intensities = actualise_les_intensites(buffers)
    return [10 * np.log10(i) for i in intensities]  # Correction de l'échelle de décibels
    
def filtrage(buf):
    buf_ = buf - np.mean(buf)
    return buf_

def get_amplitude_at_frequency(buffer, rate, freq):
    # Filtrage des données 
    buf = filtrage(buffer[-int(RESPEAKER_RATE * dt):]) 
    # Apply FFT on a dt interval 
    fft_result = np.fft.fft(buf)
    fft_freq = np.fft.fftfreq(len(buf), 1.0/rate)
    
    # Find the index of the frequency closest to the target frequency
    idx = (np.abs(fft_freq - freq)).argmin()
    
    # Return the amplitude at that frequency
    return np.abs(fft_result[idx])

def actualise_les_amplitudes_440Hz(buffers):
    freq = 440  # Frequency for A4 (440 Hz)
    amplitudes = [get_amplitude_at_frequency(buffer, RESPEAKER_RATE, freq) for buffer in buffers]
    return amplitudes


