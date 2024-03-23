import pygame
import ast

# pygame setup
pygame.init()
w = 734
h = 667
screen = pygame.display.set_mode((w, h))
running = True



# sizes & keyboard measurements
aspectratio = w/h;
key_size = [125/w, 125/h]
kb_positions = [
    [[0/w, 125/h],
     [0/w, 260/h],
     [0/w, 395/h]],
    [[135/w, 42/h],
     [135/w, 177/h],
     [135/w, 312/h]],
    [[270/w, 0/h],
     [270/w, 135/h],
     [270/w, 270/h]],
    [[405/w, 38/h],
     [405/w, 173/h],
     [405/w, 308/h],
     [540/w, 56/h],
     [540/w, 191/h],
     [540/w, 326/h]],
    [[1-key_size[0], 1-key_size[1]],
     [1-key_size[0]-135/w, 1-key_size[1]-35/h]]
]

TARGET_NAMES = [[0,0],[0,1],[0,2], [1,0],[1,1],[1,2], [2,0],[2,1],[2,2],
                [3,0],[3,1],[3,2], [3,3],[3,4],[3,5], [4,0],[4,1]]

# avoid initial bug in represantation
screen.fill("black")
pygame.display.flip()

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # fill the screen with a color to wipe away anything from last frame
    screen.fill("black")


    # read state from stdin
    state = ast.literal_eval(input())
    # check length of input state
    assert len(state) == len(TARGET_NAMES), "Can't match input"
    # echo state it on stdout
    print(state)

    # loop through all keys and draw them considering their state
    for i in range(len(TARGET_NAMES)):
        # check state and adjust color accordingly
        if state[i]:
            color = "green"
        else:
            color = "white"

        # get current position of current key
        a, b   = TARGET_NAMES[i]
        xr, yr = kb_positions[a][b]

        # draw key
        pygame.draw.rect(
            screen, color,
            (xr*w, yr*h, key_size[0]*w, key_size[1]*h)
        )

    # flip() the display to put your work on screen
    pygame.display.flip()

pygame.quit()
