#include "clocking.h"
#include "PIT.h"

static clocking_cfg_t G;
static clock_cb_t fs_cb_, rb_cb_;

bool clocking_init(const clocking_cfg_t* cfg, clock_cb_t fs_cb, clock_cb_t rb_cb)
{
    if (!cfg || !fs_cb || !rb_cb) return false;
    G = *cfg; fs_cb_ = fs_cb; rb_cb_ = rb_cb;

    bool ok0 = PIT_Init(/*ch=*/0, G.bus_hz, G.fs_hz, G.prio_fs, fs_cb_);
    bool ok1 = PIT_Init(/*ch=*/1, G.bus_hz, G.rb_hz, G.prio_rb, rb_cb_);
    return ok0 && ok1;
}

void clocking_start(void)
{
    PIT_Start(0);
    PIT_Start(1);
}

void clocking_stop(void)
{
    PIT_Stop(1);
    PIT_Stop(0);
}

bool clocking_set_fs(uint32_t fs_hz)
{
    G.fs_hz = fs_hz;
    return PIT_SetFreq(0, G.bus_hz, G.fs_hz);
}

bool clocking_set_rb(uint32_t rb_hz)
{
    G.rb_hz = rb_hz;
    return PIT_SetFreq(1, G.bus_hz, G.rb_hz);
}
