#pragma once

#include <vector>
#include <string>
#include "ProtobufPlugin.h"
#include "mesh/generated/beaconmap.pb.h"
#include "concurrency/OSThread.h"
using namespace std;


/**
 * A plugin that listens for beacons and reports RSS info to mesh
 */
class BeaconMapPlugin : public ProtobufPlugin<BeaconMap_RequestMessage>,ProtobufPlugin<Position>, private concurrency::OSThread
{
  public:
    /** Constructor
     * name is for debugging output
     */
#if(1)
    BeaconMapPlugin()
      : ProtobufPlugin<BeaconMap_RequestMessage>("BeaconRequestMsg", PortNum_PRIVATE_APP, BeaconMap_RequestMessage_fields),
        ProtobufPlugin<Position>("PositionMsg", PortNum_POSITION_APP, Position_fields),
        OSThread("BeaconMsgPlugin")
      { };
#endif

  protected:

    /** For plugin we do all of our processing in the (normally optional)
     * want_replies handling
    */
    virtual MeshPacket *allocReply() override;
     /** Called to handle a particular incoming message
    @return true if you've guaranteed you've handled this message and no other handlers should be considered for it
    */
    virtual bool handleReceivedProtobuf(const MeshPacket &mp, BeaconMap_RequestMessage *p) override;
    virtual bool handleReceivedProtobuf(const MeshPacket &mp, Position *pptr) override;
    /**
     * Periodically run this runOnce code,
     * broadcast a message if needed.
     * 
     * The method that will be called each time our thread gets a chance to run
     *
     * Returns desired period for next invocation (or RUN_SAME for no change)
     */
    virtual int32_t runOnce() override;

    vector<BeaconMap_ReplyMessage_BeaconData> BLEList;
    int32_t getBLE(vector<string> &list);
};

extern BeaconMapPlugin *beaconMapPlugin;