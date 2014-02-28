import dbus

def notify(appname, title, body, icon="", replaces_id=0, actions=[], hints={}, timeout=-1):
    bus = dbus.SessionBus()
    obj = bus.get_object('org.freedesktop.Notifications', '/org/freedesktop/Notifications')
    iface = dbus.Interface(obj, dbus_interface = 'org.freedesktop.Notifications')
    return iface.Notify(
        appname,
        replaces_id,
        icon,
        title,
        body,
        actions,
        hints,
        timeout
        )
