import tkinter as tk
import time
from tkinter import filedialog
import cv2
from PIL import Image, ImageTk
import matplotlib

# Create the main window
window = tk.Tk()
window.title("Data Collection")
window.geometry("1920x1080")
window.configure(bg="#1e1e1e")

nav = tk.Frame(window, bg="#252525", height=50)
nav.pack(fill="x")


# Initialize pages

container = tk.Frame(window)
container.pack(fill = "both", expand = True)
pages = {}

def to_Page(page_name):
    page = pages[page_name]
    page.tkraise()


# Add time for syncing

time_Title = tk.Label(nav, text = "Real Time:", font = ("Segoe UI", 24, "bold"))
time_Title.configure(bg = "#1e1e1e", fg = "white")
time_Title.grid(row = 0, column = 0, pady = 10)

tk_time = tk.Label(nav, text = "Time", font = ("Segoe UI", 24, "bold"))
tk_time.configure(bg = "#1e1e1e", fg = "white")
tk_time.grid(row = 0, column = 1, pady = 10)

def updateTime():
    pc_Time = time.strftime("%H:%M:%S") + f".{int(time.time() % 1 * 1000):03d}"
    tk_time.config(text = pc_Time)
    window.after(10, updateTime)


# Label for connection status

connection_status = tk.Canvas(nav, width=40, height=40, bg="#252525", highlightthickness=0)
connection_status.grid(row=0, column=6, padx=20)

circle = connection_status.create_oval(5, 5, 35, 35, fill="red")

def check_Connection():
    # CHANGE THIS WHEN WIFI WORKS
    current = connection_status.itemcget(circle, "fill")
    new = "green" if current == "red" else "red"
    connection_status.itemconfig(circle, fill=new)
    window.after(1000, check_Connection)

# Introduce pages for Intitial, RPM, SHOCK, and View Data

# Intitial Page

page_Initial = tk.Frame(container, bg = "lightgrey")
page_Initial.place(relwidth = 1, relheight = 1)
pages["home"] = page_Initial


# RPM Page

page_RPM = tk.Frame(container, bg = "lightgrey")
page_RPM.place(relwidth = 1, relheight = 1)
pages["RPM"] = page_RPM


# SHOCK Page

page_SHOCK = tk.Frame(container, bg = "lightgrey")
page_SHOCK.place(relwidth = 1, relheight = 1)
pages["SHOCK"] = page_SHOCK

# View Data Page

page_VIEW = tk.Frame(container, bg = "lightgrey")
page_VIEW.place(relwidth = 1, relheight = 1)
pages["VIEW"] = page_VIEW
video_area = tk.Label(page_VIEW, bg = "black")
video_area.grid(row = 1, column = 0, padx = 20, pady = 40)

    # Video Playback

cap = None

def play_video():
    global cap, frame_delay
    

    if cap is None or not cap.isOpened():
        return
    
    ret, frame = cap.read()
    if not ret:
        return
    frame = cv2.resize(frame, (320, 180))  # or any size you want
    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    img = ImageTk.PhotoImage(Image.fromarray(frame))

    video_area.config(image = img)
    video_area.image = img

    video_area.after(frame_delay, play_video)
    # Upload Video and Data file

def upload_video():
    global cap, frame_delay
    file_path = filedialog.askopenfilename(
        title="Select a Video File",
        filetypes=[("Video Files", "*.mp4 *.mov *.avi *.mkv"), ("All Files", "*.*")])
    if file_path:
        print("Selected video:", file_path)
        cap = cv2.VideoCapture(file_path) 
        fps = cap.get(cv2.CAP_PROP_FPS) # Avoid divide-by-zero 
        if fps <= 1: 
            fps = 30 
        frame_delay = int(1000 / fps)
        play_video()

def upload_data():
    file_path = filedialog.askopenfilename(
        title="Select a data File",
        filetypes=[("Data Files", "*.txt *.csv"), ("All Files", "*.*")])
    if file_path:
        print("Selected video:", file_path)
    
  




tk.Button(page_VIEW, text="Upload Video", font=("Segoe UI", 18), command=upload_video).grid(row=0, column=0, padx=20, pady=40)
tk.Button(page_VIEW, text = "Upload Data", font=("Segoe UI", 18), command=upload_data).grid(row=0, column=1, padx=20, pady=40)
page_VIEW.columnconfigure(0, weight=1)
page_VIEW.columnconfigure(1, weight=1)

# Navigation buttons

tk.Button(nav, text = "Home", font = ("Segoe UI", 18, "bold"), command = lambda: to_Page("home")).grid(row = 0, column = 2, pady = 10, padx = 10)
tk.Button(nav, text = "RPM Data", font = ("Segoe UI", 18, "bold"),command = lambda: to_Page("RPM")).grid(row = 0, column = 3, pady = 10, padx = 10)
tk.Button(nav, text = "Shock Data", font = ("Segoe UI", 18, "bold"),command = lambda: to_Page("SHOCK")).grid(row = 0, column = 4, pady = 10, padx = 10)
tk.Button(nav, text = "View Data", font = ("Segoe UI", 18, "bold"),command = lambda: to_Page("VIEW")).grid(row = 0, column = 5, pady = 10, padx = 10)

nav.columnconfigure(0, weight=0)
nav.columnconfigure(1, weight=0)
nav.columnconfigure(2, weight=1)
nav.columnconfigure(3, weight=1)
nav.columnconfigure(4, weight=1)
nav.columnconfigure(5, weight=1)
nav.columnconfigure(6, weight=1)

# Keep the window open
to_Page("home")
check_Connection()
updateTime()
window.mainloop()

