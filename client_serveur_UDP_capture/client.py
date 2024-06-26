import socket
import time
import capture_audio
import matplotlib.pyplot as plt
import numpy as np
import threading
import queue

PORT = 21000
BUFFER_SIZE = 1024
UPDATE_RATE = 5  # Nombre de fois par seconde où les graphiques sont actualisés

# Demande à l'utilisateur la liste des micros à afficher
user_input = input("Veuillez entrer la liste des micros à afficher (ex: 0,1,2,3,4,5) : ")
Indices_affiches = list(map(int, user_input.split(',')))

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
    buffers_temp = buffers
    for i in Indices_affiches:
        buf = capture_audio.filtrage(buffers_temp[i][-capture_audio.CHUNK:])
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

def udp_client(buffer_queue):
    global buffer
    sockfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_addr = ('192.168.0.107', PORT)  # Remplacez par l'adresse IP du serveur si nécessaire

    p, stream = capture_audio.open_stream()

    try:
        while True:
            buffers = capture_audio.capture_channel(stream, capture_audio.buffers)
            buffer, db_values = change_buffer(buffers)
            sockfd.sendto(buffer.encode(), server_addr)
            print(buffer + "\n")
            buffer = ""
            buffer_queue.put((buffers, db_values))
            #time.sleep(1.0 / UPDATE_RATE)
            time.sleep(capture_audio.dt)
    except Exception as e:
        print(f"Error in UDP client loop: {e}")
    finally:
        capture_audio.close_stream(stream, p)
        sockfd.close()

def plotter(buffer_queue):
    plt.ion()
    fig, (ax_signal, ax_fft) = plt.subplots(2, 1, figsize=(10, 10))
    fig_hex, ax_hex = plt.subplots(subplot_kw={'projection': 'polar'}, figsize=(6, 6))

    line_objects = [ax_signal.plot([], [], label=f'Channel {i+1}')[0] for i in range(6)]
    ax_signal.set_xlim(0, capture_audio.dt * capture_audio.buffer_size / capture_audio.RESPEAKER_RATE)
    ax_signal.set_ylim(-32768, 32767)
    ax_signal.set_title('Audio Signal')
    ax_signal.set_xlabel('Time [s]')
    ax_signal.set_ylabel('Amplitude')
    ax_signal.legend()

    while True:
        try:
            buffers, db_values = buffer_queue.get(timeout=1.0)
            plot_hexagon(ax_hex, db_values)

            # Mise à jour du signal en temps réel
            for i in Indices_affiches:
                buf = buffers[i]
                time_array = np.linspace(0, capture_audio.dt * len(buf) / capture_audio.RESPEAKER_RATE, num=len(buf))
                line_objects[i].set_data(time_array, buf)

            ax_signal.draw_artist(ax_signal.patch)
            for i in Indices_affiches:
                ax_signal.draw_artist(line_objects[i])
            fig.canvas.blit(ax_signal.bbox)
            fig.canvas.flush_events()

            # Affichage de la FFT en temps réel
            plot_fft(ax_fft, buffers, capture_audio.RESPEAKER_RATE)

            plt.draw()
        except queue.Empty:
            pass

if __name__ == "__main__":
    buffer_queue = queue.Queue()

    udp_thread = threading.Thread(target=udp_client, args=(buffer_queue,))
    plot_thread = threading.Thread(target=plotter, args=(buffer_queue,))

    udp_thread.start()
    plot_thread.start()

    udp_thread.join()
    plot_thread.join()



