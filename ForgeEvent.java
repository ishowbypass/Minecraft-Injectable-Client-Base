package client.event;

import client.feature.Module;
import client.feature.ModuleManager;
import net.minecraftforge.eventbus.api.Event;

public class ForgeEvent {
    public Event event;

    public ForgeEvent(Event event) {
        this.event = event;
        for (Module module : ModuleManager.modules) {
            if (module.enable) module.onForgeEvent(this);
        }
    }
}