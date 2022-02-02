#include "configuration.h"
#include "BeaconMapPlugin.h"
#include "MeshService.h"
#include "main.h"

#include <assert.h>

#define REPORT_INTERVAL_MSEC (30 * 1000)
#define BLE_INTERVAL_MSEC ( 1 * 1000)

BeaconMapPlugin *beaconMapPlugin;

#if(0)
MeshPacket *BeaconMapPlugin::allocReply()
{
    assert(currentRequest); // should always be !NULL
    auto req = *currentRequest;
    auto &p = req.decoded;
    // The incoming message is in p.payload
    DEBUG_MSG("Received message from=0x%0x, id=%d, msg=%.*s\n", req.from, req.id, p.payload.size, p.payload.bytes);

    screen->print("Sending reply\n");

    auto reply = ProtobufPlugin<BeaconMap_RequestMessage>::allocDataProtobuf(BeaconMap_ReplyMessage);                    // Allocate a packet for sending
    reply->decoded.payload.size = sizeof(BeaconMap_ReplyMessage); // You must specify

    return reply;
}
#endif
bool BeaconMapPlugin::handleReceivedProtobuf(const MeshPacket &req, BeaconMap_RequestMessage *pptr)
{
    auto p = *pptr;
    DEBUG_MSG("Received Beacon_RequestMsg\n");

    switch (p.typ) {
    case BeaconMap_RequestMessage_BeaconMessageType_REQUEST:
        // Print notification to LCD screen
        screen->print("Remote Beacons\n");
#if(0)
        int len = p.beaconData->size();
        for (uint8_t i = 0; i < len; i++) {
            if (p.list[i]) {
                digitalWrite(i, (p.beacons[i]));
            }
        }
#endif
        break;

    default:
        DEBUG_MSG("BeaconMap operation %d not yet implemented! FIXME\n", p.typ);
        break;
    }

    return false;
}
// recieve Position protobuf to set MYLOCATION (no GPS)
bool BeaconMapPlugin::handleReceivedProtobuf(const MeshPacket &mp, Position *pptr)
{
    auto p = *pptr;

    // If inbound message is a replay (or spoof!) of our own messages, we shouldn't process
    // (why use second-hand sources for our own data?)

    // FIXME this can in fact happen with packets sent from EUD (src=RX_SRC_USER)
    // to set fixed location, EUD-GPS location or just the time (see also issue #900)
    if (nodeDB.getNodeNum() == getFrom(&mp)) {
        DEBUG_MSG("Incoming update from MYSELF\n");
        // DEBUG_MSG("Ignored an incoming update from MYSELF\n");
        // return false;
    }

    // Log packet size and list of fields
    DEBUG_MSG("POSITION node=%08x l=%d %s%s%s%s%s%s%s%s%s%s%s%s%s%s\n", getFrom(&mp), mp.decoded.payload.size,
              p.latitude_i ? "LAT " : "", p.longitude_i ? "LON " : "", p.altitude ? "MSL " : "", p.altitude_hae ? "HAE " : "",
              p.alt_geoid_sep ? "GEO " : "", p.PDOP ? "PDOP " : "", p.HDOP ? "HDOP " : "", p.VDOP ? "VDOP " : "",
              p.sats_in_view ? "SIV " : "", p.fix_quality ? "FXQ " : "", p.fix_type ? "FXT " : "", p.pos_timestamp ? "PTS " : "",
              p.time ? "TIME " : "", p.battery_level ? "BAT " : "");

#if(0)
    if (p.time) {
        struct timeval tv;
        uint32_t secs = p.time;

        tv.tv_sec = secs;
        tv.tv_usec = 0;

        //perhapsSetRTC(RTCQualityFromNet, &tv);
    }
#endif

    nodeDB.updatePosition(getFrom(&mp), p);

    return false; // Let others look at this message also if they want
}

static uint32_t lastReportMsec=0;
int32_t BeaconMapPlugin::runOnce()
{
    if (enabled) {
        uint32_t now = millis();
        // BLEList is filled by evt_handler

        if (now - lastReportMsec >= REPORT_INTERVAL_MSEC) {
            lastReportMsec = millis();
            BeaconMap_ReplyMessage r;
            BeaconMap_ReplyMessage_MeshNodeData m = BeaconMap_ReplyMessage_MeshNodeData_init_default;
            r.nodeData = m;
            r.typ = BeaconMap_ReplyMessage_BeaconMessageType_REPLY;
            // protobuf list
            int32_t len = BLEList.size();
            //if(len)
            //{
            //    for(int i=0; i<len; i++)
            //        r.beaconData.append(BLEList[i]);
            //}
            MeshPacket *p = ProtobufPlugin<BeaconMap_RequestMessage>::allocReply();
            service.sendToMesh(p);
            BLEList.clear();
        }
    }

    return BLE_INTERVAL_MSEC; // Poll for BLE detections (FIXME, make adjustable via protobuf arg)
}