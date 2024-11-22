# Kernel-Module-Development-for-Keypad-Based-Home-Security-System

This project implements a security system on a Raspberry Pi using a kernel module and an application. It features password management via a 4x4 keypad, an LCD for static messages, and components like a buzzer, LED, and servo motor for system feedback and actions.

## ✨ Features

- **Password Management**: Set and reset passwords securely using IOCTL commands.  
- **Static LCD Display**: Displays messages ("Security System" and "Enter Password").  
- **Feedback Mechanisms**:  
  - *Correct Password*: Buzzer emits two beeps, LED lights up for 5 seconds, and servo motor unlocks.  
  - *Incorrect Password*: Buzzer sounds five consecutive beeps.  
- **Keypad Functionality**:  
  - `*`: Submit the password.  
  - `#`: Clear the current password entry.  
- **Simplified Interaction**: Password validation feedback is audio-visual only, with the LCD remaining static.

## 🛠️ Hardware Setup

1. **Raspberry Pi**: The central controller.  
2. **4x4 Keypad**: For password input.  
3. **LCD Display**: Shows static messages.  
4. **Buzzer**: Provides audio feedback.  
5. **LED Indicator**: Lights up on correct password entry.  
6. **Servo Motor**: Simulates door unlocking.  

## 📋 Steps to Use  

### 1. Set Up Project Files  
- Create a folder for the project and add the following files:  
  - `securitysystem.c`: Kernel module code.  
  - `app.c`: Application code for user interaction.  
  - `Makefile`: For compiling the project.  
  - `setup.sh`: Script to automate the setup.  

### 2. Provide Script Permissions  
Run the following command to enable execution of the setup script:  
```bash
sudo chmod 777 setup.sh
```

### 3. Execute the Setup Script  
Run the script to compile and load the kernel module:  
```bash
sudo ./setup.sh
```

### 4. Run the Application  
Interact with the system via the application. To stop the kernel module, type `3` when prompted and press `Enter`.

## 🧩 Project Structure  

```plaintext
├── securitysystem.c  # Kernel module code  
├── app.c             # Application code  
├── Makefile          # Build instructions  
└── setup.sh          # Setup automation script  
```

## 💡 How It Works  

1. **Kernel Module**: Handles interactions between components using IOCTL commands and a device file for secure password storage and validation.  
2. **Application**: Provides a user interface for setting/resetting passwords and communicating with the kernel module.  
3. **Feedback**: Combines buzzer, LED, and servo motor for immediate response to password attempts.

---

This project provides a robust framework for creating a security system on a Raspberry Pi, combining embedded systems programming with hardware control.
