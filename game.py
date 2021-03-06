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
    toplevel.geometry("%dx%d+%d+%d" % (size + (x, y)))

class App:
  def __init__(self, master):
    center(master)
    master.minsize(width=800, height=450)

    master.title("Headache")
    master.configure(background='black')

    msg = Message(master, text = help_message)
    msg.config(bg='black', foreground="white",
        justify="center", font=('times', 24, 'italic'),
        width=800)
    msg.pack()

    choices = ['Easy', 'Medium', 'Hard']
    self._gun = StringVar(root)
    self._gun.set('... Please select difficulty...')
    w = OptionMenu(root, self._gun, *choices)
    w.pack()

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
    opts = _game.Options()
    if self._gun.get() == 'Easy':
        opts.machine_gun = True
        opts.game_time = 45
        opts.ball_time = 10
    elif self._gun.get() == 'Medium':
        opts.machine_gun = True
        opts.game_time = 30
        opts.ball_time = 3
    else:
        opts.machine_gun = False
        opts.game_time = 35
        opts.ball_time = 5

    _game.run(opts)
    exit(0)

if __name__ == '__main__':
    # _game.run()
    root = Tk()
    app = App(root)
    root.mainloop()
