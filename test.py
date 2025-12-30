import tkinter as tk

window = tk.Tk()
window.title("Multi‑Page App")
window.geometry("400x300")

# --- Create a container for all pages ---
container = tk.Frame(window)
container.pack(fill="both", expand=True)

# Dictionary to store pages
pages = {}

def show_page(page_name):
    page = pages[page_name]
    page.tkraise()   # bring this page to the front

# --- Page 1 ---
page1 = tk.Frame(container, bg="lightblue")
page1.place(relwidth=1, relheight=1)
pages["page1"] = page1

tk.Label(page1, text="This is Page 1", font=("Arial", 20), bg="lightblue").pack(pady=20)
tk.Button(page1, text="Go to Page 2", command=lambda: show_page("page2")).pack()

# --- Page 2 ---
page2 = tk.Frame(container, bg="lightgreen")
page2.place(relwidth=1, relheight=1)
pages["page2"] = page2

tk.Label(page2, text="This is Page 2", font=("Arial", 20), bg="lightgreen").pack(pady=20)
tk.Button(page2, text="Go to Page 1", command=lambda: show_page("page1")).pack()

# Start on Page 1
show_page("page1")

window.mainloop()
