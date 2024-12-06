# NOTE: only for MacOS... need to test on rpi
# this is because of the menu bar / camera cutout on the macbook air
TOP_BAR_HEIGHT = 30

SNAKE_STARTING_POS = (20, 20)

BORDER_WIDTH = 6

TOP_SPEED = 10

class Player(arcade.Sprite):
    def __init__(self):
        super().__init__()
        self.size = 50
        self.texture = arcade.make_soft_square_texture(self.size, arcade.color.YELLOW, center_alpha=255, outer_alpha=55)

        self.speed_x = 1
        self.speed_y = 1

        self.life = 100
        self.last_life_loss = time.time()


        self.snake = []
        for i in range(10):
            self.snake.append((SNAKE_STARTING_POS[0] + (i * 4) * self.speed_x, SNAKE_STARTING_POS[1] + (i * 4) * self.speed_y))


    def update(self):
        # lose life every second
        if time.time() > self.last_life_loss + 1:
            self.life -= LIFE_SUCK_RATE
            self.last_life_loss = time.time()


        ### MOVEMENT AND CONFINEMENT
        head = self.snake[0]
        new_head = (head[0] + self.speed_x, head[1] + self.speed_y)

        if new_head[0] < BORDER_WIDTH:
            self.speed_x = -self.speed_x
            new_head = (BORDER_WIDTH, new_head[1])

        if new_head[0] > GAME_WINDOW.width - BORDER_WIDTH - self.size:
            self.speed_x = -self.speed_x
            new_head = (GAME_WINDOW.width - BORDER_WIDTH - self.size, new_head[1])

        if new_head[1] < BORDER_WIDTH:
            self.speed_y = -self.speed_y
            new_head = (new_head[0], BORDER_WIDTH)
        
        if new_head[1] > GAME_WINDOW.height - BORDER_WIDTH - self.size:
            self.speed_y = -self.speed_y
            new_head = (new_head[0], GAME_WINDOW.height - BORDER_WIDTH - self.size) # hmmm

        self.snake.insert(0, new_head)
        self.snake.pop()

    
    def draw(self):
        for i in range(len(self.snake) - 1):
            arcade.draw_lrwh_rectangle_textured(self.snake[i][0], self.snake[i][1], self.size - i, self.size - i, self.texture)



    def change_speed(self, delta: int) -> None:
        if self.speed_x != 0:
            self.speed_x -= delta if self.speed_x > 0 else -delta
        if self.speed_y != 0:
            self.speed_y -= delta if self.speed_y > 0 else -delta


    def change_speed_cap(self, x_delta: int = None, y_delta: int = None) -> None:
        if x_delta is not None:
            if abs(self.speed_x) > TOP_SPEED:
                self.speed_x = TOP_SPEED if self.speed_x > 0 else -TOP_SPEED # cap to TOP SPEED
                if self.speed_y != 0: # ... the goal is to reduce the X speed by 1, if not zero
                    self.speed_y -= 1 if self.speed_y > 0 else -1 # move the X speed down (towards zero)
            else:
                self.speed_x += x_delta # do the speed increase

        if y_delta is not None:
            if abs(self.speed_y) > TOP_SPEED:
                self.speed_y = TOP_SPEED if self.speed_y > 0 else -TOP_SPEED # cap to TOP SPEED
                if self.speed_x != 0: # ... the goal is to reduce the X speed by 1, if not zero
                    self.speed_x -= 1 if self.speed_x > 0 else -1 # move the X speed down (towards zero)
            else:
                self.speed_y += y_delta # do the speed increase

def on_draw(self):
    arcade.start_render()


    # draw a border around the game window
    arcade.draw_rectangle_outline(GAME_WINDOW.width / 2, GAME_WINDOW.height / 2, GAME_WINDOW.width, GAME_WINDOW.height, arcade.color.GREEN, border_width=border_width)

    # show life in top left corner
    arcade.draw_text(f"Life: {player.life}", 10, GAME_WINDOW.height * 0.9, arcade.color.WHITE, font_size=20, anchor_x="left")

    # show player x and y direction
    arcade.draw_text(f"dir_x: {player.speed_x} / dir_y: {player.speed_y}", GAME_WINDOW.width // 2, GAME_WINDOW.height * 0.9, arcade.color.YELLOW, font_size=20, anchor_x="center")

    # show pressed keys
    pressed_keys = []
    for key, cooldown_key in cooldown_keys.items():
        if cooldown_key.pressed:
            pressed_keys.append(key)
    arcade.draw_text(f"Pressed keys: {pressed_keys}", GAME_WINDOW.width - 10, GAME_WINDOW.height * 0.9, arcade.color.WHITE, font_size=20, anchor_x="right")


    player.draw()
    # player.draw_hit_box(arcade.color.BLUE)

    if paused:
        # draw pause screen
        arcade.draw_rectangle_filled(GAME_WINDOW.width / 2, GAME_WINDOW.height / 2, GAME_WINDOW.width, GAME_WINDOW.height, arcade.color.BLACK_OLIVE + (200,))
        arcade.draw_text("PAUSED", GAME_WINDOW.width / 2, GAME_WINDOW.height * 0.75, arcade.color.YELLOW_ROSE, font_size=60, anchor_x="center", anchor_y="center")

    if escape_pressed_time is not None:
        time_elapsed = time.time() - escape_pressed_time
        if time_elapsed >= HOLD_TO_QUIT_SECONDS:
            alive = False
            escape_pressed_time = None
        else:
            draw_timer_wheel(time_elapsed)





def handle_cooldown_keys(key: int = None):
    # NOTE: these can't be elif becuase this is also run in on_update() and it needs to process every one of these

    if cooldown_keys[KEY_UP].run(key=key):
        player.change_speed_cap(y_delta=1)

    if cooldown_keys[KEY_DOWN].run(key=key):
        player.change_speed_cap(y_delta=-1)

    if cooldown_keys[KEY_LEFT].run(key=key):
        player.change_speed_cap(x_delta=-1)

    if cooldown_keys[KEY_RIGHT].run(key=key):
        player.change_speed_cap(x_delta=1)
