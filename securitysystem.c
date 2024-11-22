#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

#define ROWS 4
#define COLS 4
#define PASSWORD "1234"
#define PASSWORD_MAX_LENGTH 5
#define LED_PIN 17
#define BUZZER_PIN 26
#define SERVO_PIN 13

#define RS_PIN 18
#define E_PIN 23
#define D4_PIN 24
#define D5_PIN 25
#define D6_PIN 8
#define D7_PIN 7

// Servo PWM settings
#define SERVO_MIN_PULSE 500
#define SERVO_MAX_PULSE 2400
#define SERVO_FREQ 50

#define IOCTL_SET_PASSWORD _IOW('a', 1, char *)
#define IOCTL_RESET_PASSWORD _IO('a', 2)

static unsigned int row_pins[ROWS] = {4, 9, 10, 11};
static unsigned int col_pins[COLS] = {5, 6, 27, 22};

static char keypad_map[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

static char current_password[PASSWORD_MAX_LENGTH] = PASSWORD; // Default password
static struct timer_list keypad_timer;
static int scan_interval = 200; // in milliseconds
static char entered_password[PASSWORD_MAX_LENGTH];
static int current_index = 0;

static int major_number;
void lcd_send_command(char cmd);
void lcd_send_data(char data);
void lcd_init(void);
void lcd_print(const char *str);

static void beep_buzzer(int times, int duration) {
    int i;
    for (i = 0; i < times; i++) {
        gpio_set_value(BUZZER_PIN, 1);
        mdelay(duration);
        gpio_set_value(BUZZER_PIN, 0);
        if (i < times - 1) {
            mdelay(duration);
        }
    }
}

void servo_pwm(int angle) {
    int pulse_width = SERVO_MIN_PULSE + (angle * (SERVO_MAX_PULSE - SERVO_MIN_PULSE) / 180);
    gpio_set_value(SERVO_PIN, 1);
    udelay(pulse_width);
    gpio_set_value(SERVO_PIN, 0);
    udelay(20000 - pulse_width);
}

static void scan_keypad(struct timer_list *t) {
    int row, col;
    for (row = 0; row < ROWS; row++) {
        for (int r = 0; r < ROWS; r++) {
            gpio_set_value(row_pins[r], 1);
        }
        gpio_set_value(row_pins[row], 0);
        for (col = 0; col < COLS; col++) {
            if (!gpio_get_value(col_pins[col])) {
                char key_pressed = keypad_map[row][col];
                beep_buzzer(1, 200);

                if (key_pressed == '*') {
                    entered_password[current_index] = '\0';
                    if (strcmp(entered_password, current_password) == 0) {
                        beep_buzzer(2, 100);
                        gpio_set_value(LED_PIN, 1);
                        servo_pwm(130);
                        mdelay(5000);
                        servo_pwm(30);
                        gpio_set_value(LED_PIN, 0);
                    } else {
                        beep_buzzer(5, 200);
                    }
                    current_index = 0;
                } else if (key_pressed == '#') {
                    current_index = 0;
                    memset(entered_password, 0, sizeof(entered_password));
                    beep_buzzer(1, 1000);
                } else if (current_index < 4) {
                    entered_password[current_index++] = key_pressed;
                }
            }
        }
    }
    mod_timer(&keypad_timer, jiffies + msecs_to_jiffies(scan_interval));
}

void lcd_init(void) {
    gpio_request(RS_PIN, "RS_PIN");
    gpio_request(E_PIN, "E_PIN");
    gpio_request(D4_PIN, "D4_PIN");
    gpio_request(D5_PIN, "D5_PIN");
    gpio_request(D6_PIN, "D6_PIN");
    gpio_request(D7_PIN, "D7_PIN");

    gpio_direction_output(RS_PIN, 0);
    gpio_direction_output(E_PIN, 0);
    gpio_direction_output(D4_PIN, 0);
    gpio_direction_output(D5_PIN, 0);
    gpio_direction_output(D6_PIN, 0);
    gpio_direction_output(D7_PIN, 0);

    mdelay(15);
    lcd_send_command(0x33);
    lcd_send_command(0x32);
    lcd_send_command(0x28);
    lcd_send_command(0x0C);
    lcd_send_command(0x01);
    mdelay(2);
}

void lcd_send_command(char cmd) {
    gpio_set_value(RS_PIN, 0);
    gpio_set_value(D4_PIN, (cmd >> 4) & 1);
    gpio_set_value(D5_PIN, (cmd >> 5) & 1);
    gpio_set_value(D6_PIN, (cmd >> 6) & 1);
    gpio_set_value(D7_PIN, (cmd >> 7) & 1);
    gpio_set_value(E_PIN, 1);
    mdelay(1);
    gpio_set_value(E_PIN, 0);
    mdelay(1);
    gpio_set_value(D4_PIN, cmd & 1);
    gpio_set_value(D5_PIN, (cmd >> 1) & 1);
    gpio_set_value(D6_PIN, (cmd >> 2) & 1);
    gpio_set_value(D7_PIN, (cmd >> 3) & 1);
    gpio_set_value(E_PIN, 1);
    mdelay(1);
    gpio_set_value(E_PIN, 0);
    mdelay(1);
}

void lcd_print(const char *str) {
    while (*str) {
        gpio_set_value(RS_PIN, 1);
        gpio_set_value(D4_PIN, (*str >> 4) & 1);
        gpio_set_value(D5_PIN, (*str >> 5) & 1);
        gpio_set_value(D6_PIN, (*str >> 6) & 1);
        gpio_set_value(D7_PIN, (*str >> 7) & 1);
        gpio_set_value(E_PIN, 1);
        mdelay(1);
        gpio_set_value(E_PIN, 0);
        mdelay(1);
        gpio_set_value(D4_PIN, *str & 1);
        gpio_set_value(D5_PIN, (*str >> 1) & 1);
        gpio_set_value(D6_PIN, (*str >> 2) & 1);
        gpio_set_value(D7_PIN, (*str >> 3) & 1);
        gpio_set_value(E_PIN, 1);
        mdelay(1);
        gpio_set_value(E_PIN, 0);
        mdelay(1);
        str++;
    }
}

static long securitysystem_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    char new_password[PASSWORD_MAX_LENGTH];

    switch (cmd) {
        case IOCTL_SET_PASSWORD:
            if (copy_from_user(new_password, (char __user *)arg, PASSWORD_MAX_LENGTH)) {
                return -EFAULT;
            }
            strncpy(current_password, new_password, PASSWORD_MAX_LENGTH);
            printk(KERN_INFO "Password updated successfully.\n");
            break;

        case IOCTL_RESET_PASSWORD:
            strncpy(current_password, PASSWORD, PASSWORD_MAX_LENGTH);
            printk(KERN_INFO "Password reset to default.\n");
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .unlocked_ioctl = securitysystem_ioctl,
};

static int securitysystem_init(void) {
    int ret, i;

    major_number = register_chrdev(0, "securitysystem", &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Failed to register character device\n");
        return major_number;
    }
    printk(KERN_INFO "Registered character device with major number %d\n", major_number);

    lcd_init();
    lcd_print("Security system");
    lcd_send_command(0xC0);
    lcd_print("Enter password");

    ret = gpio_request(LED_PIN, "LED_PIN");
    if (ret) {
        printk(KERN_ALERT "Failed to request GPIO %d for LED\n", LED_PIN);
        return ret;
    }
    gpio_direction_output(LED_PIN, 0);

    ret = gpio_request(BUZZER_PIN, "BUZZER_PIN");
    if (ret) {
        printk(KERN_ALERT "Failed to request GPIO %d for Buzzer\n", BUZZER_PIN);
        return ret;
    }
    gpio_direction_output(BUZZER_PIN, 0);

    ret = gpio_request(SERVO_PIN, "SERVO_PIN");
    if (ret) {
        printk(KERN_ALERT "Failed to request GPIO %d for servo\n", SERVO_PIN);
        return ret;
    }
    gpio_direction_output(SERVO_PIN, 0);

    for (i = 0; i < ROWS; i++) {
        ret = gpio_request(row_pins[i], "Row Pin");
        if (ret) {
            printk(KERN_ALERT "Failed to request GPIO %d for row pin\n", row_pins[i]);
            return ret;
        }
        gpio_direction_output(row_pins[i], 1);
    }
    for (i = 0; i < COLS; i++) {
        ret = gpio_request(col_pins[i], "Col Pin");
        if (ret) {
            printk(KERN_ALERT "Failed to request GPIO %d for column pin\n", col_pins[i]);
            return ret;
        }
        gpio_direction_input(col_pins[i]);
    }

    timer_setup(&keypad_timer, scan_keypad, 0);
    mod_timer(&keypad_timer, jiffies + msecs_to_jiffies(scan_interval));

    return 0;
}

static void securitysystem_exit(void) {
    int i;
    del_timer(&keypad_timer);
    unregister_chrdev(major_number, "securitysystem");
    gpio_free(LED_PIN);
    gpio_free(BUZZER_PIN);
    gpio_free(SERVO_PIN);
    for (i = 0; i < ROWS; i++) {
        gpio_free(row_pins[i]);
    }
    for (i = 0; i < COLS; i++) {
        gpio_free(col_pins[i]);
    }
    lcd_send_command(0x01);
}

module_init(securitysystem_init);
module_exit(securitysystem_exit);
