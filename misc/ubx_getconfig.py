#!/usr/bin/python3
#
# Polls uBlox for its current configuration
#
# Format should be comparable to uCenter-generated files
# 

import time
import sys
import os

SERIALDEVICE='/dev/ttyLP2'

GPS_SERIAL_CONFIG_CMD="/bin/stty -F /dev/ttyLP2 115200 raw -echo -echoe -echok"

ACK_CLASS_ID = 0x05
CFG_CLASS_ID = 0x06

# Debugging
debug = False

# Global junk char count
cnt_skipmsg = 0

def read_ubx_msg(ser):
    timeout_not_reached = True
    msg_stage = 0
    while timeout_not_reached:
        if msg_stage == 0:
            xx = ord(os.read(ser,1))
            # Detect start of a UBX message
            if xx == 0xb5:
                msg_stage = 1
            # Detect NMEA command - 0x24 + 0x47 + (N|L|P) - the NMEA command starting chars
            elif xx == 0x24:
                xx = ord(os.read(ser,1))
                if xx == 0x47:
                    xx = ord(os.read(ser,1))
                    if xx == 'N' or xx == 'L' or xx == 'P':
                        # Read all of the nmea, tossing it
                        while xx != 0x0d and xx != 0x0a and xx != 0xb5:
                            xx = ord(os.read(ser,1))
                # If we ended above with start of a UBX msg, set msg_stage
                if xx == 0xb5:
                    msg_stage = 1
        # Get the second ubx marker char
        if msg_stage == 1:
            xx = ord(os.read(ser,1))
            if xx == 0x62:
                msg_stage = 2
            else:
                if debug:
                    print('Abort! Got a %02X after a b5, reverting to stage 0' % xx)
                msg_stage = 0
        # Read the msg
        if msg_stage == 2:
            ubx_msg_classid = ord(os.read(ser,1))
            ubx_msg_msgid   = ord(os.read(ser,1))
            body_len_low    = ord(os.read(ser,1))
            body_len_high   = ord(os.read(ser,1))
            ubx_msg_bodylen = body_len_high * 256 + body_len_low
            # Read message
            #   Note: OLD WAY, big reads, had garbage included in the 300+ len messages
            #   Single char at a time way - those reads of 308 work now!
            ubx_msg_body = []
            for x in range(ubx_msg_bodylen):
                ubx_msg_body.append(ord(os.read(ser,1)))
            csum1           = ord(os.read(ser,1))
            csum2           = ord(os.read(ser,1))
            # Validate the msg
            vsum1, vsum2 = get_mod256_fletcher16([ubx_msg_classid,
                                                        ubx_msg_msgid,
                                                        body_len_low,
                                                        body_len_high] + ubx_msg_body)
            if csum1 != vsum1 or csum2 != vsum2:
                print('ERROR: Checksum Failure! %02X vs %02X and %02X vs %02X' % (csum1,vsum1,csum2,vsum2))
            return ubx_msg_classid, ubx_msg_msgid, ubx_msg_bodylen, ubx_msg_body


def get_mod256_fletcher16(data: bytes):
    """
    Accepts a string as input.
    Returns the Fletcher16 checksum value and its two 8-bit components
    8-bit implementation (16-bit checksum)
    NOTE: This is fletcher16 but with a 256 modulus, not 255 (wiki reference implementation)
    """
    sum1, sum2 = int(), int()
    for x in data:
        sum1 = (sum1 + x) % 256
        sum2 = (sum2 + sum1) % 256
    return sum1, sum2


# Our desired UBX-CFG-* messages and their msg_id's
CFG_NAMES_AND_IDS = (
    ('CFG-ANT', 0x13),
    ('CFG-DAT', 0x06),
    ('CFG-GEOFENCE', 0x69),
    ('CFG-GNSS', 0x3E),
    ('CFG-INF', 0x02,       #<-- Special, is x2
                ([0],[1])),     #  - One byte payload, for protocolID
    ('CFG-ITFM', 0x39),
    ('CFG-LOGFILTER', 0x47),
    ('CFG-MSG', 0x01,      #<-- This one is special special, is x65 !!
        (                  #   - Two byte payloads, msgClass + msgID
            (0x01, 0x01),
            (0x01, 0x02),
            (0x01, 0x03),
            (0x01, 0x04),
            (0x01, 0x06),
            (0x01, 0x07),
            (0x01, 0x09),
            (0x01, 0x11),
            (0x01, 0x12),
            (0x01, 0x20),
            (0x01, 0x21),
            (0x01, 0x22),
            (0x01, 0x23),
            (0x01, 0x24),
            (0x01, 0x25),
            (0x01, 0x26),
            (0x01, 0x30),
            (0x01, 0x31),
            (0x01, 0x32),
            (0x01, 0x34),
            (0x01, 0x35),
            (0x01, 0x36),
            (0x01, 0x39),
            (0x01, 0x60),
            (0x01, 0x61),
            (0x02, 0x13),
            (0x02, 0x14),
            (0x02, 0x15),
            (0x02, 0x20),
            (0x02, 0x23),
            (0x02, 0x59),
            (0x02, 0x61),
            (0x0A, 0x02),
            (0x0A, 0x06),
            (0x0A, 0x07),
            (0x0A, 0x08),
            (0x0A, 0x09),
            (0x0A, 0x0B),
            (0x0A, 0x0D),
            (0x0A, 0x21),
            (0x0A, 0x2B),
            (0x0B, 0x01),
            (0x0B, 0x30),
            (0x0B, 0x31),
            (0x0B, 0x33),
            (0x0D, 0x01),
            (0x0D, 0x03),
            (0x0D, 0x06),
            (0x21, 0x08),
            (0xF0, 0x00),
            (0xF0, 0x01),
            (0xF0, 0x02),
            (0xF0, 0x03),
            (0xF0, 0x04),
            (0xF0, 0x05),
            (0xF0, 0x06),
            (0xF0, 0x07),
            (0xF0, 0x08),
            (0xF0, 0x09),
            (0xF0, 0x0A),
            (0xF0, 0x0D),
            (0xF0, 0x0F),
            (0xF1, 0x00),
            (0xF1, 0x03),
            (0xF1, 0x04),
        )),
    ('CFG-NAV5', 0x24),
    ('CFG-NAVX5', 0x23),
    ('CFG-NMEA', 0x17),
    ('CFG-ODO', 0x1E),
    ('CFG-OTP', 0x41),
    ('CFG-PM2', 0x3B),
    ('CFG-PMS', 0x86),
    ('CFG-PRT', 0x00,       #<-- Special, is x4
                            #   -- One byte payload, for PortID
                            #   -- We have four valid UART port IDs
             ([0x00],[0x01],[0x03],[0x04])),
    ('CFG-PWR', 0x57),
    ('CFG-RATE', 0x08),
    ('CFG-RINV', 0x34),
    ('CFG-RXM', 0x11),
    ('CFG-SBAS', 0x16),
    ('CFG-TP5', 0x31,      #<-- is x2
                           #  -- One byte payload, tpIdx - Time pulse selection
            ([0],[1])), #  (0 = TIMEPULSE, 1 = TIMEPULSE2)
    ('CFG-USB', 0x1B)
)


# Builds and send UBlox a request message for the passed msg_id and payload
# Payload is a list of ints, or an empty list
def send_cfg_req(cfg_msg_id, payload):
    req = [
        0xb5, 0x62,             # Header
        CFG_CLASS_ID,           # msgClass - 6 = UBX-CFG
        cfg_msg_id              # msgID
    ]
    # Add length of payload, in little endian
    req.append(len(payload) % 256)
    req.append(len(payload) // 256)
    # Add payload
    req += payload
    # Compute Checksum and append it to req
    vsum1, vsum2 = get_mod256_fletcher16(req[2:])
    req += (vsum1, vsum2)
    # Write it out
    os.write(ser, bytes(req))


def print_cfg_response(cfg_name, msg_class_id, msg_id, msg_bodylen, msg_body):
    if len(msg_body) != msg_bodylen:
        print('# WARNING: %s has bodylen=%d but msg_body has %d bytes' % (cfg_name, msg_bodylen, len(msg_body)))
    # Print name + classid + msgid
    print('%s - %02X %02X' % (cfg_name, msg_class_id, msg_id), end='')
    # Print body length as two bytes in little endian order
    print(' %02X %02X' % (msg_bodylen % 256, msg_bodylen // 256), end='')
    # Print body
    for x in msg_body:
        print(' %02X' % x, end='')
    print('')


def get_desired_ubx_msg(want_class_id, want_msg_id, timeout_secs):
    global cnt_skipmsg
    start_time = time.time()
    done = False
    while not done:
        got_class_id, got_msg_id, got_bodylen, got_body = read_ubx_msg(ser)
        if got_class_id == want_class_id and got_msg_id == want_msg_id:
            return True, got_bodylen, got_body
        elif (time.time() - start_time) > timeout_secs:
            done = True
        else:
            cnt_skipmsg += 1
            if debug:
                print('Skipping msg %d - %02X-%02X' % (cnt_skipmsg, got_class_id, got_msg_id))
    return False, None, None

def get_any_of_desired_ubx_msg(desired_pairs, timeout_secs):
    global cnt_skipmsg
    start_time = time.time()
    done = False
    while not done:
        got_class_id, got_msg_id, got_bodylen, got_body = read_ubx_msg(ser)
        for want_class_id, want_msg_id in desired_pairs:
            if got_class_id == want_class_id and got_msg_id == want_msg_id:
                return True, got_class_id, got_msg_id, got_bodylen, got_body
        if (time.time() - start_time) > timeout_secs:
            done = True
        else:
            cnt_skipmsg += 1
            if debug:
                print('Skipping msg %d - %02X-%02X' % (cnt_skipmsg, got_class_id, got_msg_id))
    return False, None, None, None, None





# Config and open uBlox Serial device
os.system(GPS_SERIAL_CONFIG_CMD)
ser = os.open(SERIALDEVICE, os.O_RDWR)


# Main Loop
cnt_missed_acks = 0
cnt_acks = 0
cnt_naks = 0
cnt_got_cfg = 0
cnt_missed_cfg = 0
poll_status = dict([(x[0],0) for x in CFG_NAMES_AND_IDS])
for want_cfg in CFG_NAMES_AND_IDS:
    want_name, want_msg_id = want_cfg[:2]
    # Most poll messages have no payloads, meaning there is only
    #   one poll for that particular config.  However, four of them
    #   have payloads (CFG-MSG has 65!), meaning there are multiple
    #   variations of the poll message for that config, and we need to
    #   iterate sending and receiving each variation.   If there is
    #   a third parameter in want_cfg, it contains a list of the
    #   payloads for each variation.  We set req_paylist_list to that.
    if len(want_cfg) == 3:
        req_payload_list = want_cfg[2]
    # However, if there isn't a third component, only one msg, with no payload
    # We'll make the payload list be a single, empty payload
    else:
        req_payload_list = [[]]
    # Iterate through the req_payload_list, sending polls and receive the data
    for i, payload in enumerate(req_payload_list):
        if debug:
            if len(req_payload_list) > 1:
                print('Requesting %s %d/%d -- %02X-%02X + %s' % \
                    (want_name, 
                     i+1, len(req_payload_list),
                     CFG_CLASS_ID, want_msg_id,
                     ' '.join([('%02X' % x) for x in payload])))
            else:
                print('Requesting %s -- %02X-%02X' % (want_name, CFG_CLASS_ID, want_msg_id))
        send_cfg_req(want_msg_id, payload)

        flag_received_config = False

        # Get the request ack/nak *OR* the cfg-response
        got_cfg_response = False
        x = get_any_of_desired_ubx_msg(((ACK_CLASS_ID, 0x01),
                                        (ACK_CLASS_ID, 0x00),
                                        (CFG_CLASS_ID, want_msg_id)), 2.0)
        if x[0]:
            got_class_id, got_msg_id, msg_bodylen, msg_body = x[1:]
            if got_class_id == CFG_CLASS_ID:
                if debug: print('Got cfg-response!')
                print_cfg_response(want_name, got_class_id, got_msg_id, msg_bodylen, msg_body)
                got_cfg_response = True
                poll_status[want_name] = 1
                flag_received_config = True
            elif got_class_id == ACK_CLASS_ID:
                if got_msg_id == 0:
                    if debug: print('Got NAK acknowledgement - BOO')
                    cnt_naks += 1
                elif got_msg_id == 1:
                    if debug: print('Got ACK! YAY')
                    cnt_acks += 1
                else:
                    print('# WARNING: Got Weird UBX-ACK-%02X response!' % got_msg_id)

        # Get cfg-response if we didnt get it before the ack
        if got_cfg_response == False:
            got_msg, msg_bodylen, msg_body = get_desired_ubx_msg(CFG_CLASS_ID, want_msg_id, 2.0)
            if got_msg:
                print_cfg_response(want_name, CFG_CLASS_ID, want_msg_id, msg_bodylen, msg_body)
                poll_status[want_name] = 1
                flag_received_config = True
            else:
                if debug:
                    print('Failed to get msg %s' % want_name)

        # Did we get all we requested - todo: handle getting some but not all of the payload-requests
        if flag_received_config:
            cnt_got_cfg +=1
        else:
            cnt_missed_cfg += 1

if debug:
    print('Summary:')
    print('   %d retrieved configs' % cnt_got_cfg)
    print('   %d configs were not retrieved' % cnt_missed_cfg)
    print('   %d ACKs, %d NAKs' % (cnt_acks, cnt_naks))
    print('Configs that were not retrieved:')
    print('   ',end='')
    print('  '.join([x for x in sorted(poll_status.keys()) if poll_status[x] == 0]))
