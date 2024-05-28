import socket
import time
import capture_audio
import matplotlib.pyplot as plt
import numpy as np

PORT = 21000
BUFFER_SIZE = 1024
UPDATE_RATE = 5  # Nombre de fois par seconde où les graphiques sont actualisés

# Déclaration du buffer global
buffer = "hello\n"

def change_buffer(buffers):
    #intensities = capture_audio.actualise_les_intensites(buffers)
    intensities = capture_audio.actualise_les_amplitudes_440Hz(buffers)
    buf = "Vecteur :" + " " + ", ".join(map(str, intensities)) + ";"
    return buf, intensities

def plot_hexagon(ax, db_values):
    angles = np.linspace(0, 2 * np.pi, 6, endpoint=False).tolist()
    angles += angles[:1]
    
    db_values += db_values[:1]

    ax.clear()
    ax.set_theta_offset(np.pi / 2)
    ax.set_theta_direction(-1)

    ax.set_xticks(angles[:-1])
    ax.set_xticklabels(["Mic1", "Mic2", "Mic3", "Mic4", "Mic5", "Mic6"])

    ax.plot(angles, db_values, linewidth=1, linestyle='solid')
    ax.fill(angles, db_values, 'b', alpha=0.1)

def plot_fft(ax, buffers, rate):
    ax.clear()
    VOICE_FREQ_MIN = 50
    VOICE_FREQ_MAX = 1000
    for i, buffer in enumerate(buffers):
        buf = capture_audio.filtrage(buffer[-int(capture_audio.RESPEAKER_RATE * capture_audio.dt):])
        fft_result = np.fft.fft(buf)
        fft_freq = np.fft.fftfreq(len(buf), 1.0/rate)
        
        indices = np.where((fft_freq >= VOICE_FREQ_MIN) & (fft_freq <= VOICE_FREQ_MAX))
        fft_freq = fft_freq[indices]
        fft_result = np.abs(fft_result[indices])
        
        ax.plot(fft_freq, fft_result, label=f'Channel {i+1}')
    
    ax.set_title('FFT of Audio Signal')
    ax.set_xlabel('Frequency [Hz]')
    ax.set_ylabel('Amplitude')
    ax.legend()

def client():
    global buffer
    sockfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_addr = ('192.168.0.107', PORT)  # Remplacez par l'adresse IP du serveur si nécessaire

    plt.ion()
    fig, (ax_signal, ax_fft) = plt.subplots(2, 1, figsize=(10, 10))
    fig_hex, ax_hex = plt.subplots(subplot_kw={'projection': 'polar'}, figsize=(6, 6))

    p, stream = capture_audio.open_stream()
    update_count = 0  # Compteur pour limiter la fréquence de mise à jour des graphiques

    try:
        line_objects = [ax_signal.plot([], [], label=f'Channel {i+1}')[0] for i in range(6)]
        ax_signal.set_xlim(0, capture_audio.dt * capture_audio.buffer_size / capture_audio.RESPEAKER_RATE)
        ax_signal.set_ylim(-32768, 32767)
        ax_signal.set_title('Audio Signal')
        ax_signal.set_xlabel('Time [s]')
        ax_signal.set_ylabel('Amplitude')
        ax_signal.legend()
        
        while True:
            buffers = capture_audio.capture_channel(stream, capture_audio.buffers)
            buffer, db_values = change_buffer(buffers)
            sockfd.sendto(buffer.encode(), server_addr)
            print(buffer + "\n")
            buffer = ""

            # Limiter la fréquence de mise à jour des graphiques
            if update_count % (capture_audio.RESPEAKER_RATE // (capture_audio.CHUNK * UPDATE_RATE)) == 0:
                plot_hexagon(ax_hex, db_values)

                # Mise à jour du signal en temps réel
                time_array = np.linspace(0, capture_audio.dt * capture_audio.buffer_size / capture_audio.RESPEAKER_RATE, num=capture_audio.buffer_size)
                for i, buf in enumerate(buffers):
                    line_objects[i].set_data(time_array, buf)

                ax_signal.draw_artist(ax_signal.patch)
                for line in line_objects:
                    ax_signal.draw_artist(line)
                fig.canvas.blit(ax_signal.bbox)
                fig.canvas.flush_events()

                # Affichage de la FFT en temps réel
                plot_fft(ax_fft, buffers, capture_audio.RESPEAKER_RATE)

                plt.draw()

            update_count += 1

    except Exception as e:
        print(f"Error in client loop: {e}")
    finally:
        capture_audio.close_stream(stream, p)
    
    sockfd.close()

# Appel de la fonction client pour démarrer le client
client()


