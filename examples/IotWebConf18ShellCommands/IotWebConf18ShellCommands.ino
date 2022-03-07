/**
 * Example: IotWebConf18ShellCommands.ino
 * Description:
 *   This example is a modified IotWebConf01Minimal.ino, with added shell commands.
 *
 *   Simply type "help" via Serial in order to see the available commands:

      > help
      'help' not supported. Available commands :
        ap 0/1 (Enables/disables access point).
        ap_pwd abc (Sets AP password to abc).
        name abc (Sets ThingName to abc).
        reset (Restarts the ESP).
        reset_config (Resets the complete IotWeb config).
        save_config (Saves the config to EEPROM).
        ssid name (Sets SSID to name).
        wifi 0/1 (Enables/disables WiFi).
        wifi_pwd abc (Sets WiFi password to abc).

  * The commands allow you to setup the thing directly via Serial,
  * without the need to type long passwords on a smartphone.
  *
  * You can copy-paste multiple commands at once, separated by a newline.
  * They will be executed one after the other.

      > name my_thing
      Calling : name('my_thing')
      Setting Thing name to my_thing

      > wifi_pwd my_wifi_password
      Calling : wifi_pwd('my_wifi_password')
      Setting WiFi password to my_wifi_password

      > ssid my_wifi
      Calling : ssid('my_wifi')
      Setting WiFi ssid to my_wifi

      > ap_pwd p4ssw0rd
      Calling : ap_pwd('p4ssw0rd')
      Setting AP password to p4ssw0rd

      > save_config
      Calling : save_config()

      Config version: init
      Config size: 165
      Saving configuration
      [iwcAll]
      |-- [iwcSys]
      |   |-- 'iwcThingName' with value: 'my_thing'
      |   |-- 'iwcApPassword' with value: <hidden>
      |   |-- [iwcWifi0]
      |   |   |-- 'iwcWifiSsid' with value: 'my_wifi'
      |   |   \-- 'iwcWifiPassword' with value: <hidden>
      |   \-- 'iwcApTimeout' with value: '30'
      |-- [iwcCustom]
      \-- [hidden]

      State changing from: 2 to 3
      Connecting to [my_wifi] (password is hidden)
      WiFi timeout (ms): 30000
      ...
      ...
 *
 */

#include <IotWebConf.h>

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "testThing";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "smrtTHNG8266";

// -- Method declarations.
void handleRoot();

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);

/*****************************************************************************
 * Shell interface.
 *
 * (Could be moved to shell.h)
 ****************************************************************************/
namespace shell {
  void defineCommand(const char *name, void (*function)(), const __FlashStringHelper *doc_fstring);
  void defineIntCommand(const char *name, void (*function)(int32_t), const __FlashStringHelper *doc_fstring);
  void defineStringCommand(const char *name, void (*function)(char*), const __FlashStringHelper *doc_fstring);

  void checkSerialInput();

  void execute(const char *command_line);
}

/*****************************************************************************
 * Define functions which can be called in the shell.
 ****************************************************************************/

void resetConfig(){
  Serial.println(F("Resetting config..."));
  iotWebConf.getRootParameterGroup()->applyDefaultValue();
  Serial.println(F("Done!"));
}

void setSSID(char *ssid){
  Serial.print(F("Setting WiFi ssid to "));
  Serial.println(ssid);
  strlcpy(iotWebConf.getWifiSsidParameter()->valueBuffer, ssid, iotWebConf.getWifiSsidParameter()->getLength());
}

void setWifiPassword(char *pwd){
  Serial.print(F("Setting WiFi password to "));
  Serial.println(pwd);
  strlcpy(iotWebConf.getWifiPasswordParameter()->valueBuffer, pwd, iotWebConf.getWifiPasswordParameter()->getLength());
}

void setApPassword(char *pwd){
  Serial.print(F("Setting AP password to "));
  Serial.println(pwd);
  strlcpy(iotWebConf.getApPasswordParameter()->valueBuffer, pwd, iotWebConf.getApPasswordParameter()->getLength());
}

void setThingName(char *name){
  Serial.print(F("Setting Thing name to "));
  Serial.println(name);
  strlcpy(iotWebConf.getThingNameParameter()->valueBuffer, name, iotWebConf.getThingNameParameter()->getLength());
}

void apOnOff(int onOff){
  if (onOff) {
    Serial.print(F("Enable "));
  } else {
    Serial.print(F("Disable "));
  }
  Serial.println(F("AP mode!"));
  iotWebConf.forceApMode(onOff);
}

void wifiOnOff(int onOff){
  if (onOff) {
    Serial.print(F("Enable Wifi!"));
    iotWebConf.goOnLine();
  } else {
    Serial.print(F("Disable Wifi!"));
    iotWebConf.goOffLine();
  }
}

/*****************************************************************************
 * Define shell commands with name, function and documentation.
 * The commands accept either 0 argument, one string or one integer.
 *
 * Feel free to add other commands, e.g. to set custom parameters.
 * The second argument can be either a function name or a lambda.
 ****************************************************************************/

void defineShellCommands() {
  shell::defineCommand("reset", []() { ESP.restart(); }, F("(Restarts the ESP)"));
  shell::defineCommand("save_config", []() { iotWebConf.saveConfig(); }, F("(Saves the config to EEPROM)"));
  shell::defineCommand("reset_config", resetConfig, F("(Resets the complete IotWeb config)"));

  shell::defineStringCommand("ssid", setSSID, F("name (Sets SSID to name)"));
  shell::defineStringCommand("wifi_pwd", setWifiPassword, F("abc (Sets WiFi password to abc)"));
  shell::defineStringCommand("ap_pwd", setApPassword, F("abc (Sets AP password to abc)"));
  shell::defineStringCommand("name", setThingName, F("abc (Sets ThingName to abc)"));

  shell::defineIntCommand("ap", apOnOff, F("0/1 (Enables/disables access point)"));
  shell::defineIntCommand("wifi", wifiOnOff, F("0/1 (Enables/disables WiFi)"));
}

/*****************************************************************************
 * Arduino sketch
 ****************************************************************************/

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Starting up IotWebConf18ShellCommands..."));

  // -- Initializing the configuration.
  iotWebConf.init();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

  defineShellCommands();

  Serial.println(F("Ready."));
}

void loop()
{
  iotWebConf.doLoop();
  shell::checkSerialInput();
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  const char* s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>"
    "<title>IotWebConf 18 Shell</title></head><body>"
    "Go to <a href='config'>configure page</a> to change settings."
    "</body></html>\n";

  server.send(200, "text/html", s);
}

/*****************************************************************************
 * Shell logic.
 *
 * (Could be moved to shell.cpp)
 ****************************************************************************/

namespace shell {
  const uint8_t MAX_COMMANDS = 10;
  const uint8_t MAX_COMMAND_SIZE = 40;

  uint8_t commands_count = 0;

  enum input_type {
    NONE,
    INT32,
    STRING
  };

  struct Command {
    const char *name;
    union {
      void (*voidFunction)();
      void (*intFunction)(int32_t);
      void (*strFunction)(char*);
    };
    const char *doc;
    input_type parameter_type;
  };

  struct CommandLine {
    char function_name[MAX_COMMAND_SIZE];
    input_type argument_type;
    int32_t int_argument;
    char str_argument[MAX_COMMAND_SIZE];
  };

  Command commands[MAX_COMMANDS];

  bool addCommand(const char *name, const __FlashStringHelper *doc_fstring) {
    if (commands_count < MAX_COMMANDS) {
      commands[commands_count].name = name;
      commands[commands_count].doc = (const char*) doc_fstring;
      return true;
    } else {
      Serial.println(F("Too many commands have been defined."));
      return false;
    }
  }

  void defineCommand(const char *name, void (*function)(), const __FlashStringHelper *doc_fstring) {
    if (addCommand(name, doc_fstring)) {
      commands[commands_count].voidFunction = function;
      commands[commands_count++].parameter_type = NONE;
    }
  }

  void defineIntCommand(const char *name, void (*function)(int32_t), const __FlashStringHelper *doc_fstring) {
    if (addCommand(name, doc_fstring)) {
      commands[commands_count].intFunction = function;
      commands[commands_count++].parameter_type = INT32;
    }
  }

  void defineStringCommand(const char *name, void (*function)(char*), const __FlashStringHelper *doc_fstring) {
    if (addCommand(name, doc_fstring)) {
      commands[commands_count].strFunction = function;
      commands[commands_count++].parameter_type = STRING;
    }
  }

  /*
   * Tries to split a string command (e.g. 'mqtt 60' or 'show_csv') into
   * a CommandLine struct (function_name, argument_type and argument)
   */
  void parseCommand(const char *command, CommandLine &command_line) {
    if (strlen(command) == 0) {
      Serial.println(F("Received empty command"));
      command_line.argument_type = NONE;
      return;
    }

    char *first_space;
    first_space = strchr(command, ' ');

    if (first_space == NULL) {
      command_line.argument_type = NONE;
      strlcpy(command_line.function_name, command, MAX_COMMAND_SIZE);
      return;
    }

    strlcpy(command_line.function_name, command, first_space - command + 1);
    strlcpy(command_line.str_argument, first_space + 1, MAX_COMMAND_SIZE - (first_space - command) - 1);

    char *end;
    command_line.int_argument = strtol(command_line.str_argument, &end, 0); // Accepts 123 or 0xFF00FF

    if (*end) {
      command_line.argument_type = STRING;
    } else {
      command_line.argument_type = INT32;
    }
  }

  int compareCommandNames(const void *s1, const void *s2) {
    struct Command *c1 = (struct Command*) s1;
    struct Command *c2 = (struct Command*) s2;
    return strcmp(c1->name, c2->name);
  }

  void listAvailableCommands() {
    qsort(commands, commands_count, sizeof(commands[0]), compareCommandNames);
    for (uint8_t i = 0; i < commands_count; i++) {
      Serial.print(F("  "));
      Serial.print(commands[i].name);
      Serial.print(F(" "));
      Serial.print(commands[i].doc);
      Serial.println(F("."));
    }
  }

  /*
   * Saves bytes from Serial.read() until enter is pressed, and tries to run the corresponding command.
   *   http://www.gammon.com.au/serial
   */
  void processSerialInput(const byte input_byte) {
    static char input_line[MAX_COMMAND_SIZE];
    static unsigned int input_pos = 0;
    switch (input_byte) {
    case '\n': // end of text
      Serial.println();
      input_line[input_pos] = 0;
      execute(input_line);
      input_pos = 0;
      break;
    case '\r': // discard carriage return
      break;
    case '\b': // backspace
      if (input_pos > 0) {
        input_pos--;
        Serial.print(F("\b \b"));
      }
      break;
    default:
      if (input_pos == 0) {
        Serial.print(F("> "));
      }
      // keep adding if not full ... allow for terminating null byte
      if (input_pos < (MAX_COMMAND_SIZE - 1)) {
        input_line[input_pos++] = input_byte;
        Serial.print((char) input_byte);
      }
      break;
    }
  }

  void checkSerialInput() {
    while (Serial.available() > 0) {
      shell::processSerialInput(Serial.read());
    }
  }

  /*
   * Tries to find the corresponding callback for a given command. Name and parameter type should fit.
   */
  void execute(const char *command_str) {
    CommandLine input;
    parseCommand(command_str, input);
    for (uint8_t i = 0; i < commands_count; i++) {
      if (!strcmp(input.function_name, commands[i].name) && input.argument_type == commands[i].parameter_type) {
        Serial.print(F("Calling : "));
        Serial.print(input.function_name);
        switch (input.argument_type) {
        case NONE:
          Serial.println(F("()"));
          commands[i].voidFunction();
          return;
        case INT32:
          Serial.print(F("("));
          Serial.print(input.int_argument);
          Serial.println(F(")"));
          commands[i].intFunction(input.int_argument);
          return;
        case STRING:
          Serial.print(F("('"));
          Serial.print(input.str_argument);
          Serial.println(F("')"));
          commands[i].strFunction(input.str_argument);
          return;
        }
      }
    }
    Serial.print(F("'"));
    Serial.print(command_str);
    Serial.println(F("' not supported. Available commands :"));
    listAvailableCommands();
  }
}
