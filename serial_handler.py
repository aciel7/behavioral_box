import serial
import csv
import time
import numpy as np
import sounddevice as sd

# ——— Configuration ———
SERIAL_PORT   = '/dev/ttyACM0'
BAUD_RATE     = 115200
CSV_FILE      = 'states.csv'

LE_TONE_FREQ  = 440.0   # Hz for low-effort tone
HE_TONE_FREQ  = 880.0   # Hz for high-effort tone
TONE_DURATION = 0.5     # seconds

def play_tone(freq, duration):
    """Generate and play a sine wave at `freq` Hz for `duration` seconds."""
    fs = 44100  # sampling rate
    t = np.linspace(0, duration, int(fs * duration), endpoint=False)
    wave = np.sin(2 * np.pi * freq * t)
    sd.play(wave, fs)
    sd.wait()

def main():
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

    with open(CSV_FILE, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['timestamp', 'state'])
        print(f"Logging states to {CSV_FILE}. Press Ctrl+C to stop.")

        while True:
            raw = ser.readline().decode('utf-8', errors='replace').strip()
            if not raw:
                continue

            if raw.startswith('state='):
                ts    = time.time()
                state = raw.split('=', 1)[1]

                writer.writerow([ts, state])
                csvfile.flush()
                print(f"{ts:.3f}: {state}")

                if state == 'le_tone':
                    play_tone(LE_TONE_FREQ, TONE_DURATION)
                elif state == 'he_tone':
                    play_tone(HE_TONE_FREQ, TONE_DURATION)

if __name__ == '__main__':
    main()               