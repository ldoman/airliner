from pyliner_exceptions import InvalidStateError
from sensor import Sensor, SensorAccess


class Service(Sensor):
    """Services are long-running passive components to Pyliner.

    A service may be started or stopped at any time by the program. In the event
    that a service is stopped, it should cease any auxiliary tasks such as
    threads or listening to events that are not critical to the component.

    Services are not granted control access to the vehicle.

    Lifecycle:
        attach -> start -> stop -> detach
    """
    STARTED = 'STARTED'

    def start(self):
        """Called when a service is started.

        The service should start any threads or processes that it runs here.
        """
        if self._state is not Service.ATTACHED:
            raise InvalidStateError('Service must be stopped before starting.')
        self._state = Service.STARTED

    def stop(self):
        """Called when a service is stopped.

        The service should stop any threads or processes that it started.
        """
        if self._state is not Service.STARTED:
            raise InvalidStateError('Service must be started before stopping.')
        self._state = Service.ATTACHED


class ServiceAccess(SensorAccess):
    def start(self):
        self._component.start()

    def stop(self):
        self._component.stop()