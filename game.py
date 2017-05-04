from game import _game
from tkinter import *

help_message = """---- Game with doom heads ----
As the game title suggests, in this game player finds himself in extremely
hostile environment completely surrounded by aggressive heads. Players task
is to kill them all by throwing metal balls. If head catches you, you die.
If you run out of time, you die. Have fun.
"""

def center(toplevel):
    toplevel.update_idletasks()
    w = toplevel.winfo_screenwidth()
    h = toplevel.winfo_screenheight()
    size = tuple(int(_) for _ in toplevel.geometry().split('+')[0].split('x'))
    x = w/2 - size[0]/2
    y = h/2 - size[1]/2
    print(size + (x, y))
    toplevel.geometry("%dx%d+%d+%d" % (size + (x, y)))

class App:
  def __init__(self, master):
    center(master)
    master.minsize(width=800, height=400)
    master.maxsize(width=800, height=400)

    master.title("Headache")
    master.configure(background='black')

    msg = Message(master, text = help_message)
    msg.config(bg='black', foreground="white",
        justify="center", font=('times', 24, 'italic'),
        width=800)
    msg.pack( )

    frame = Frame(master)
    frame.pack()
    self.button = Button(frame,
                         text="Exit", fg="red",
                         justify="right",
                         command=quit)
    self.button.pack(side=LEFT)
    self.slogan = Button(frame,
                         text="New Game",
                         justify="left",
                         command=self.run_game)
    self.slogan.pack(side=LEFT)
  def run_game(self):
    _game.run()

if __name__ == '__main__':
    root = Tk()
    app = App(root)
    root.mainloop()
