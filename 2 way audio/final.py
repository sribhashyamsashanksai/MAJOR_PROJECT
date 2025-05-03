from flask import Flask, Response, request
import cv2
import sounddevice as sd
import wavio
from gtts import gTTS
import os
import urllib.request
import speech_recognition as sr
import pygame
import RPi.GPIO as GPIO
import threading

app = Flask(__name__)
camera = cv2.VideoCapture(0)  # OpenCV camera setup
GPIO.setwarnings(False)
# GPIO setup
GPIO.setmode(GPIO.BCM)
GPIO.setup(2, GPIO.IN, pull_up_down=GPIO.PUD_UP)  # Button connected to GPIO4 (BCM mode)

# Function to record audio
def record_audio():
    recognizer = sr.Recognizer()

    # Listen for input
    with sr.Microphone() as source:
        print("Listening...")
        audio = recognizer.listen(source)
    
    try:
        # Convert audio to text
        prompt = recognizer.recognize_google(audio)
        print("You said:", prompt)
        # URL encode the prompt for sending to ThingSpeak
        prompt_encoded = urllib.parse.quote(prompt)
        # Send the recognized text to ThingSpeak
        urllib.request.urlopen("https://api.thingspeak.com/update?api_key=YHMDJ1GZMBZ2J0HV&field1=" + prompt_encoded)
        return prompt
    except sr.UnknownValueError:
        return "Could not understand audio"
    except sr.RequestError:
        return "Could not request results from Google Speech Recognition"

# Function to detect GPIO button press in a separate thread
def gpio_event_listener():
    while True:
        input_state = GPIO.input(2)  # Read the state of the button (GPIO4)
        if input_state == GPIO.LOW:  # Button is pressed (active-low)
            print("Button pressed, recording audio...")
            record_audio()  # Call the record_audio function when button is pressed

# Route for video streaming
def generate_frames():
    while True:
        success, frame = camera.read()
        if not success:
            break
        else:
            # Encode frame as JPEG
            ret, buffer = cv2.imencode('.jpg', frame)
            frame = buffer.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

@app.route('/video_feed')
def video_feed():
    # Video streaming route
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

# Route to receive text and convert to speech
@app.route('/text_to_speech')
def text_to_speech():
    # Retrieve text from ThingSpeak
    read_link = 'https://api.thingspeak.com/channels/109073/fields/1/last?api_key=3UE4UAR11LXBD8ZZ'
    response = urllib.request.urlopen(read_link)
    received_text = response.readline().decode()
    print("Received text:", received_text)

    # Convert text to speech
    tts = gTTS(text=received_text, lang='en')
    tts.save("response_audio.mp3")
    
    # Play audio using pygame
    pygame.mixer.init()
    pygame.mixer.music.load("response_audio.mp3")
    pygame.mixer.music.play()

    # Wait until the audio playback finishes
    while pygame.mixer.music.get_busy():
        continue

    return received_text

# Run the server with a separate thread for GPIO button listening
if __name__ == "__main__":
    # Start GPIO event listener in a separate thread to monitor the button press
    gpio_thread = threading.Thread(target=gpio_event_listener)
    gpio_thread.daemon = True  # Allow thread to exit when the main program exits
    gpio_thread.start()

    # Run Flask app
    app.run(host="0.0.0.0", port=5000)
