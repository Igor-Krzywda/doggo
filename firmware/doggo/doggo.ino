#include <Wire.h>
#include <Rtc_Pcf8563.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>

#include "include/csv.h"
#include "include/debug.h"
#include "include/rtc.h"

const char MSG_USERNAME_EXISTS[] = "NAZWA ISTNIEJE\0",
           MSG_YOUR_NUMBER_IS[] = "NUMER KONTA TO:\0",
           MSG_INPUT_USER_NUMBER[] = "NUMER KONTA:\0",
           MSG_INPUT_PIN[] = "PIN:\0",
           MSG_INPUT_USERNAME[] = "PSEUDONIM:\0",
           MSG_NO_SIGNED_USER[] = "BRAK KONTA\0",
           MSG_WRONG_CODE[] = "ZLY KOD!\0",
           MSG_ADDING_USER_0[] = "ZAKLADANIE\0",
           MSG_ADDING_USER_1[] = "NOWEGO_KONTA\0",
           MSG_LOGGING_IN[] = "LOGOWANIE\0",
           MSG_STARTING_WALK_0[] = "CZAS DAC PSIAKOM\0",
           MSG_STARTING_WALK_1[] = "SZCZESCIE!\0",
           MSG_ENDING_WALK_0[] = "DZIEKUJEMY ZA\0",
           MSG_ENDING_WALK_1[] = "TWOJ CZAS!\0";

const uint8_t UPDATE_TIME = 't',
              DUMP_SD = 'd';

const uint8_t CHIP_SELECT = 10;

//            BUTTON LAYOUT
//            =============
//                BTN_1
//        
//        BTN_0           BTN_2    BTN_4
//                
//                BTN_3
//

const uint8_t BUTTONS[5] = {7,5,4,6,3};

const char ROOT[] = "/doggo\0";
const char USERBASE[] = "doggo/usrs_v2.csv\0";
char LOG_FILE[SIZE_FILENAME];

unsigned new_user_id = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);
Rtc_Pcf8563 rtc;


void setup()
{
    for(uint8_t i = 0; i < 5; ++i) { pinMode(BUTTONS[i], INPUT); }

    Serial.begin(115200);

    if(!SD.begin(CHIP_SELECT))
    {
        Serial.println(F("Initialization failed!"));
        while(1);
    }

    Serial.println(F("Initialization successful!"));
    Serial.println(rtc.formatTime());

    lcd.init();                   
    lcd.backlight();

    new_user_id = count_users();
    get_log_filename(&rtc, LOG_FILE);
}

void loop()
{
    if(digitalRead(BUTTONS[1]))
    {
        delay(200);
        add_new_user();
    }
    else if(digitalRead(BUTTONS[0]))
    {
        delay(200);
        log_in(true);
    }
    else if(digitalRead(BUTTONS[2]))
    {
        delay(200);
        log_in(false);
    }
    else if(digitalRead(BUTTONS[4]))
    {
        delay(200);

        File f = SD.open("/");
        DEBUG_dump_sd(SD.open("/"), 0);
        f.close();

        Serial.print("\ndone\n");
    }
    else if(Serial.available() > 0)
    {
        exec_cmd(Serial.read());
    }

    idle();
}

void add_new_user()
{
    char username[SIZE_INPUT_STR],
         pin_code[SIZE_INPUT_NUM],
         user_id[SIZE_INPUT_NUM],
         record[SIZE_RECORD];
    
    lcd_prompt(MSG_ADDING_USER_0, MSG_ADDING_USER_1, 1000); 
    lcd_display(MSG_INPUT_USERNAME, "");
    read_input(username, SIZE_INPUT_STR, false);

    if(get_record_by_field(USERBASE, username, 
                           record, SIZE_RECORD))
    {
        lcd_prompt(MSG_USERNAME_EXISTS, "\0", 500);
        return;
    }

    lcd_display(MSG_INPUT_PIN, "");
    read_input(pin_code, SIZE_INPUT_NUM, true);
  
    sprintf(user_id, "#%d\0", new_user_id);

    sprintf(record, "%s,%s,%s\n\0", 
            user_id, pin_code, username);

    append_record(USERBASE, record);
    ++new_user_id;

    lcd_prompt(MSG_YOUR_NUMBER_IS, user_id, 10000);
}

void log_in(bool start)
{
    char input[SIZE_INPUT_NUM],
         user_id[SIZE_INPUT_NUM],
         timestamp[SIZE_TIMESTAMP],
         record[SIZE_RECORD];
 
    lcd_prompt(MSG_LOGGING_IN, "", 1000);
    lcd_display(MSG_INPUT_USER_NUMBER, "");
    read_input(input, SIZE_INPUT_NUM, true);

    sprintf(user_id, "#%s", input);

    if(!get_record_by_field(USERBASE, user_id, 
                           record, SIZE_RECORD))
    {
        lcd_prompt(MSG_NO_SIGNED_USER, "\0", 500);
        return;
    }

    lcd_display(MSG_INPUT_PIN, "");
    read_input(input, SIZE_INPUT_NUM, true);

    if(!find_field(input, record))
    {
        lcd_prompt(MSG_WRONG_CODE, "\0", 500);
        return;
    }

    get_timestamp(&rtc, timestamp);
    sprintf(record, "%s,%s,%d\n\0", user_id, timestamp, (start) ? 1 : 0);
    append_record(LOG_FILE, record);

    if(start)
    {
        lcd_prompt(MSG_STARTING_WALK_0, MSG_STARTING_WALK_1, 1000);
    }else
    {
        lcd_prompt(MSG_ENDING_WALK_0, MSG_ENDING_WALK_1, 1000);
    }
}


void read_input(char buffer_input[],
                size_t len,
                bool read_num)
{
    char c_start = read_num ? '0' : 'A',
         c_rollback = read_num ? '9' : 'Z',
         c_current = c_start;
    bool input_read = false;
    uint8_t i = 0;
    
    do
    {
        lcd.cursor();
        lcd.setCursor(i,1);
        lcd.write(c_current);

        if(digitalRead(BUTTONS[1]))         // GO CHARACTER UP
        {
            delay(200);
            c_current = (c_current < c_rollback)
                ? c_current + 1 
                : c_start;
        }
        else if(digitalRead(BUTTONS[3]))    // GO CHARACTER DOWN
        {
            delay(200);
            c_current = (c_current > c_start)
                ? c_current - 1 
                : c_rollback;
        }
        else if(digitalRead(BUTTONS[0]))    // BACKSPACE
        {
            delay(200);
            lcd.write(' ');

            c_current = ' ';
            if(i > 0) i--;
        }
        else if(digitalRead(BUTTONS[2]))    // ACCEPT CHARACTER
        {
            delay(200);

            buffer_input[i] = c_current;
            c_current = c_start;

            if(i < len) ++i;

            lcd.setCursor(i, 1);
        }
        else if(digitalRead(BUTTONS[4]))    // ACCEPT INPUT
        {
            delay(200);
            
            buffer_input[i] = '\0';
            input_read = true;
            lcd.clear();
        }
    } while(!input_read);
}

unsigned count_users()
{
    unsigned number = 0;
    char c;
    File f = SD.open(USERBASE, FILE_READ);

    while((c = f.read()) != EOF)
    {
        if(c == '\n') number++;
    }
    
    return number;
}

void lcd_prompt(char row_upper[], 
                char row_lower[], 
                unsigned time)
{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(row_upper);

    lcd.setCursor(0,1);
    lcd.print(row_lower);

    delay(time);
    lcd.clear();
}

void lcd_display(char row_upper[], 
                char row_lower[])
{
    lcd.setCursor(0,0);
    lcd.print(row_upper);

    lcd.setCursor(0,1);
    lcd.print(row_lower);
}

void exec_cmd(uint8_t cmd)
{
    switch(cmd)
    {
        case UPDATE_TIME:
            update_time(&rtc);
            break;
        case DUMP_SD:
            dump_contents();
            break;
        default: 
            break;
    }
}

void dump_contents()
{
    lcd_display("Dumping files...", "");
    dump_through_serial(ROOT);
    Serial.write('|');
    lcd_prompt("Done!", "", 1000);
}

void idle()
{
    lcd.setCursor(0,0);
    lcd.print(rtc.formatTime());

    lcd.setCursor(0,1);
    lcd.print(rtc.formatDate());
}
