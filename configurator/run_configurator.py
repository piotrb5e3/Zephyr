#!/usr/sbin/env python3
from cmd import Cmd
import argparse
from configurator import Configurator, ConfiguratorException


class ConfiguratorCMD(Cmd):
    configurator = None
    intro = ("Welcome to the Zephyr configurator. "
             "Use command '?' for help text.")
    prompt = ">"

    def __init__(self, args):
        super(ConfiguratorCMD, self).__init__()
        self.configurator = Configurator(args)

    def do_set_id(self, args):
        """Usage: set_id ID
Set chip ID
"""
        try:
            splitargs = args.split()
            id_to_set = int(splitargs[0])
            self.configurator.set_id(id_to_set)
            print("OK")
        except ConfiguratorException as e:
            print(str(e))
        except ValueError:
            print("Usage: set_id ID")
        except IndexError:
            print("Usage: set_id ID")

    def do_set_ip(self, args):
        """Usage: set_ip IP
Set the IP this chip will connect to
"""
        try:
            ip = args.split()[0]
            ip = ip.split(".")
            self.configurator.set_ip(ip)
            print("OK")
        except ConfiguratorException as e:
            print(str(e))
        except IndexError:
            print("Usage: set_ip IP")

    def do_set_port(self, args):
        """Usage: set_port PORT
Set the TCP PORT this chip will connect to. Must be < 65536
"""
        try:
            splitargs = args.split()
            port = int(splitargs[0])
            self.configurator.set_port(port)
            print("OK")
        except ConfiguratorException as e:
            print(str(e))
        except ValueError:
            print("Usage: set_port PORT")
        except IndexError:
            print("Usage: set_port PORT")

    def do_reset_nonce(self, args):
        """Usage: reset_nonce
Set the internal nonce counter to all 0s.
"""
        try:
            self.configurator.reset_nonce()
            print("OK")
        except ConfiguratorException as e:
            print(str(e))
        except ValueError:
            print("Usage: setid PORT")
        except IndexError:
            print("Usage: setid PORT")

    def do_set_key(self, args):
        """Usage: set_key
Generate and set a random HMAC key used for uthentication with the server.
"""
        try:
            key = self.configurator.set_key()
            print(key)
            print("OK")
        except ConfiguratorException as e:
            print(str(e))

    def do_set_location(self, args):
        """Usage: set_location LOCATION
Set the LOCATION on the server where the chip will POST to.
"""
        try:
            location = args.split()[0]
            self.configurator.set_location(location)
            print("OK")
        except ConfiguratorException as e:
            print(str(e))
        except IndexError:
            print("Usage: set_location LOCATION")
            
    def do_set_ssid(self, args):
        """Usage: set_ssid SSID
Set the SSID of the WIFi this chip will connect to.
"""
        try:
            ssid = args.split()[0]
            self.configurator.set_ssid(ssid)
            print("OK")
        except ConfiguratorException as e:
            print(str(e))
        except IndexError:
            print("Usage: set_ssid SSID")
            
    def do_set_password(self, args):
        """Usage: set_password PASSWORD
Set the PASSWORD of the WIFi this chip will connect to.
"""
        try:
            passwd = args.split()[0]
            self.configurator.set_password(passwd)
            print("OK")
        except ConfiguratorException as e:
            print(str(e))
        except IndexError:
            print("Usage: set_password PASSWORD")

    def do_read_conf(self, args):
        """Usage: read_conf
Set the LOCATION on the server where the chip will POST to.
        """
        try:
            conf = self.configurator.read_conf()
            print(
                """Conf:
* ID: {id}
* IP: {ip}
* Port: {port}
* Key: {key}
* Location: {location}
* SSID: {ssid}
* Password: {password}
* Time: {time}""".format(**conf))
            print("OK")
        except ConfiguratorException as e:
            print(str(e))

    def do_set_time(self, args):
        """Usage: set_time [TZ]
Set the RTC chip time to match host time.
This can take up to 1 minute due to need for synchronization.
"""
        try:
            ar = args.split()
            if len(ar) > 0:
                self.configurator.set_rtc(offset=ar[0])
            else:
                self.configurator.set_rtc()
            print("OK")

        except ConfiguratorException as e:
            print(str(e))

    def do_configure_rtc(self, args):
        """Usage: configure_rtc
Set correct configuration values in RTC chip
"""
        try:
            self.configurator.configure_rtc()
            print("OK")
        except ConfiguratorException as e:
            print(str(e))

    def do_EOF(self, args):
        print("")
        print("Bye.")
        return True


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='PlantDuino configurator')
    parser.add_argument(
        '-p', '--port', dest='port', metavar='PORT', default='/dev/ttyACM0',
        help='serial port with arduino connected to it')
    parser.add_argument(
        '-b', '--baud', dest='baud', metavar='BAUD', type=int, default=115200,
        help='UART baud rate')

    ConfiguratorCMD(parser.parse_args()).cmdloop()
