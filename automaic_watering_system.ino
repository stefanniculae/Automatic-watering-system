#define PUMP_PIN 2
#define PUSH_BUTTON_PIN 5
#define OUTPUT_ENCODER_A 6
#define OUTPUT_ENCODER_B 7

extern volatile unsigned long timer0_millis;

int number_of_seconds = 0;
int number_of_hours = 0;
int push_button_state = 0;
int output_a_state;
int output_a_previous_state;
int change_number_of_watering_seconds = 0;
int change_number_of_waiting_hours = 0;
int ready_to_set_something = 0;
int out_of_setting_function = 1;
unsigned long current_millis = 0;
unsigned long last_millis = 0;
unsigned long seconds__watering_millis = 0;
unsigned long hours_waiting_millis = 0;
int in_action = 0;
int exit_from_set_loop = 0;

void setup()
{
  pinMode(OUTPUT_ENCODER_A, INPUT);
  pinMode(OUTPUT_ENCODER_B, INPUT);
  pinMode(PUSH_BUTTON_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);

  Serial.begin (9600);
  output_a_previous_state = digitalRead(OUTPUT_ENCODER_A);
}

void loop()
{
  status_check();
  if (ready_to_set_something == 1)
  {
    out_of_setting_function = 0;
    watering_seconds_preset();
    seconds__watering_millis = 1000 * number_of_seconds;
    out_of_setting_function = 1;
    status_check();
    waiting_hours_preset();
    hours_waiting_millis = 1000 * 1 * number_of_hours;  //for hours 1 -> 3600 /// for prototype we put 1 to work with seconds
    ready_to_set_something = 0;
    out_of_setting_function = 1;
    the_machine_is_set();
    reset_clock();
  }

  after_set_actions();
  in_action = 0;
}

void after_set_actions()
{
  while (in_action == 1)
  {
    watering();
    if (exit_from_set_loop == 1)
    {
      break;
    }
    waiting();
    if (exit_from_set_loop == 1)
    {
      break;
    }
    reset_clock();
  }
}

void reset_clock()
{
  noInterrupts ();
  timer0_millis = 0;
  interrupts ();
}

void watering()
{
  last_millis = millis();
  current_millis = millis();
  digitalWrite(PUMP_PIN, HIGH);
  while (last_millis + seconds__watering_millis > current_millis)
  {
    check_push_button_state();
    if (push_button_state == 1)
    {
      exit_from_set_loop = 1;
      break;
    }
    current_millis = millis();
  }
  digitalWrite(PUMP_PIN, LOW);
}

void waiting()
{
  last_millis = millis();
  current_millis = millis();
  while (last_millis + hours_waiting_millis > current_millis)
  {
    check_push_button_state();
    if (push_button_state == 1)
    {
      exit_from_set_loop = 1;
      break;
    }
    current_millis = millis();
  }
}

void the_machine_is_set()
{
  while (push_button_state == 1)
  {
    check_push_button_state();
  }
  in_action = 1;
}

void status_check()
{
  check_push_button_state();
  while (out_of_setting_function == 1 && push_button_state == 1)
  {
    push_button_state = digitalRead(PUSH_BUTTON_PIN);
    ready_to_set_something = 1;
  }
}

void check_push_button_state()
{
  push_button_state = digitalRead(PUSH_BUTTON_PIN);
}

void watering_seconds_preset()
{
  while (push_button_state == 0)
  {
    output_a_state = digitalRead(OUTPUT_ENCODER_A);
    if (output_a_state != output_a_previous_state)
    {
      if (digitalRead(OUTPUT_ENCODER_B) != output_a_state)
      {
        if (number_of_seconds < 999)
        {
          number_of_seconds++;
          Serial.print("Seconds: ");
          Serial.println(number_of_seconds);
        }
      }
      else
      {
        if (number_of_seconds > 0)
        {
          number_of_seconds--;
          Serial.print("Seconds: ");
          Serial.println(number_of_seconds);
        }
      }
    }
    output_a_previous_state = output_a_state;
    check_push_button_state();
  }
}

void waiting_hours_preset()
{
  while (push_button_state == 0)
  {
    output_a_state = digitalRead(OUTPUT_ENCODER_A);
    if (output_a_state != output_a_previous_state)
    {
      if (digitalRead(OUTPUT_ENCODER_B) != output_a_state)
      {
        if (number_of_hours < 999)
        {
          number_of_hours++;
          Serial.print("Hours: ");
          Serial.println(number_of_hours);
        }
      }
      else
      {
        if (number_of_hours > 0)
        {
          number_of_hours--;
          Serial.print("Hours: ");
          Serial.println(number_of_hours);
        }
      }
    }
    output_a_previous_state = output_a_state;
    check_push_button_state();
  }
}
