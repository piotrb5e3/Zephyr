import binascii
import time as tm
from datetime import datetime, timezone, timedelta
import serial as ps

OK = "OK"
ERR = "ERR"
LINE_END = "\n"

CMD_START_CHAR = '@'
CMD_END_CHAR = ';'

FAILSTR = "!"
OKSTR = "+"
SEPSTR = ","

SET_RTC_CMD = 'T'
CONFIGURE_RTC_CMD = 'R'
SET_KEY_CMD = 'K'
SET_ID_CMD = 'D'
SET_IP_CMD = 'A'
SET_PORT_CMD = 'P'
RESET_IV_CTR_CMD = 'I'
SYNC_CMD = 'S'
SET_LOCATION_CMD = 'L'
GET_CONF_CMD = 'V'

EXPECTED_CONF_COUNT = 7


class Configurator(object):
    args = None
    serial = None

    def __init__(self, args):
        self.args = args
        self.serial = ps.Serial(port=args.port, baudrate=args.baud, timeout=5)
        tm.sleep(2)
        self.sync()

    def wait_response(self):
        c = self._read().decode()
        while c not in [OKSTR, FAILSTR]:
            c = self._read().decode()
        if c == OKSTR:
            return OK
        else:
            return ERR

    def set_id(self, id_num):
        self._writeln(CMD_START_CHAR + SET_ID_CMD + str(id_num) + CMD_END_CHAR + LINE_END)
        if self.wait_response() != OK:
            l = self._readln()
            raise ConfiguratorException("Set ID failed: " + l)

    def set_ip(self, ip):
        try:
            num_ip = map(int, ip)
            if any(map((lambda x: x < 0), num_ip)) or any(map((lambda x: x > 255), num_ip)):
                raise ValueError()
        except ValueError:
            raise ConfiguratorException("Not a valid IP: " + ".".join(ip))

        self._writeln(CMD_START_CHAR + SET_IP_CMD + ".".join(ip) + CMD_END_CHAR)
        if self.wait_response() != OK:
            l = self._readln()
            raise ConfiguratorException("Set IP failed: " + l)

    def set_port(self, port):
        if port < 1 or port >= 65536:
            raise ConfiguratorException("Not a valid port number: " + str(port))

        self._writeln(CMD_START_CHAR + SET_PORT_CMD + str(port) + CMD_END_CHAR)
        if self.wait_response() != OK:
            l = self._readln()
            raise ConfiguratorException("Set port failed: " + l)

    def reset_nonce(self):
        self._writeln(CMD_START_CHAR + RESET_IV_CTR_CMD + CMD_END_CHAR)
        if self.wait_response() != OK:
            l = self._readln()
            raise ConfiguratorException("Nonce counter reset failed: " + l)

    def set_key(self, hex_key):
        if len(hex_key) != 32:  # 16 bytes
            raise ConfiguratorException("Invalid key size, expected 32 hex characters")
        try:
            _ = binascii.unhexlify(hex_key)
        except binascii.Error as e:
            raise ConfiguratorException("Invalid key: " + str(e))

        self._writeln(CMD_START_CHAR + SET_KEY_CMD + hex_key + CMD_END_CHAR)
        if self.wait_response() != OK:
            l = self._readln()
            raise ConfiguratorException("Set key failed: " + l)

    def set_location(self, location):
        if ";" in location:
            raise ConfiguratorException("Invalid character: ';'")
        if "," in location:
            raise ConfiguratorException("Invalid character: ','")
        if len(location) > (256 - 32 - 1):  # check that it fits in 223
            raise ConfiguratorException("Location too long. Must be 223 characters or less.")
        self._writeln(CMD_START_CHAR + SET_LOCATION_CMD + location + CMD_END_CHAR)
        if self.wait_response() != OK:
            l = self._readln()
            raise ConfiguratorException("Set location failed: " + l)

    def read_conf(self):
        self._writeln(CMD_START_CHAR + GET_CONF_CMD + CMD_END_CHAR)
        if self.wait_response() != OK:
            l = self._readln()
            raise ConfiguratorException("Read conf failed: " + l)

        conf = self._readln().split(',')
        if len(conf) != EXPECTED_CONF_COUNT:
            raise ConfiguratorException("Read conf failed: unexpected conf parameters count")

        return {
            'id': conf[0],
            'ip': conf[1],
            'port': conf[2],
            'key': conf[3],
            'nonce_ctr': conf[4],
            'location': conf[5],
            'time': conf[6],
        }

    def set_rtc(self, offset=None):
        delta = timedelta()
        if offset:
            try:
                offset_sgn = offset[0]
                offset = int(offset[1:])
            except ValueError:
                raise ConfiguratorException("Incorrect offset value")
            except IndexError:
                raise ConfiguratorException("Incorrect offset value")
            if offset_sgn not in ["+", "-"]:
                raise ConfiguratorException("Incorrect offset value")
            if offset_sgn == "-":
                offset *= -1
            delta = timedelta(hours=offset)
        print("Waiting...")
        dt = datetime.now(tz=timezone.utc)
        while dt.second != 0:
            tm.sleep(0.1)
            dt = datetime.now(tz=timezone.utc)
        dt += delta
        cmd_body = "{yr},{month},{day},{weekday},{hr},{mn}".format(
            yr=dt.year % 100, month=dt.month, day=dt.day, weekday=dt.weekday(), hr=dt.hour, mn=dt.minute)

        print("Attempting set...")
        self._writeln(CMD_START_CHAR + SET_RTC_CMD + cmd_body + CMD_END_CHAR)
        if self.wait_response() != OK:
            l = self._readln()
            raise ConfiguratorException("Time set failed: " + l)

    def configure_rtc(self):
        self._writeln(CMD_START_CHAR + CONFIGURE_RTC_CMD + CMD_END_CHAR)
        if self.wait_response() != OK:
            l = self._readln()
            raise ConfiguratorException("RTC configuration failed: " + l)

    def sync(self):
        self._writeln(CMD_START_CHAR + SYNC_CMD + CMD_END_CHAR)
        if self.wait_response() != OK:
            l = self._readln()
            raise ConfiguratorException("Sync failed: " + l)

    def _writeln(self, s):
        bs_to_send = (s + LINE_END).encode()
        self.serial.write(bs_to_send)

    def _read(self, size=1):
        r = self.serial.read(size)
        if len(r) < size:
            raise ConfiguratorException("Connection timed out!")
        return r

    def _readln(self):
        l = self.serial.readline().decode()
        if l.endswith("\n"):
            return l
        raise ConfiguratorException("Connection timed out!")


class ConfiguratorException(Exception):
    pass
