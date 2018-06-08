from collections import Container
from numbers import Real


from enum import Enum

from navigation.control import odd


class Direction(Enum):
    CLOCKWISE = 1
    NEAREST = 0
    COUNTERCLOCKWISE = -1


class Heading(float):
    """Subclass of float bounded [0, 360)."""
    def __new__(cls, value=0.0):
        # Can't wrap inside init because by then self is already set.
        return super(Heading, cls).__new__(cls, value % 360)

    def __add__(self, other):
        if isinstance(other, Real):
            return Heading(float(self) + other)
        raise NotImplemented

    def __radd__(self, other):
        return self + other

    def __sub__(self, other):
        if isinstance(other, Real):
            return Heading(float(self) - other)
        raise NotImplemented

    def __rsub__(self, other):
        return self - other

    @staticmethod
    def distance(h1, h2, direction=Direction.NEAREST, underflow=0.0):
        """Return the number of degrees difference between headings.

        Value is positive for clockwise and negative for counterclockwise
        differences. If a direction is given the distance will be in that
        direction.

        If underflow is passed it is the allowable amount of error that the
        distance is allowed to return a value not of the appropriate sign. This
        may be useful in the case of overshooting a turn and not wanting to go
        all the way around again.
        """
        if 0 > h1 > 360 or 0 > h2 > 360:
            raise ValueError('h1 and h2 must be [0, 360).')

        diff = float(h2) - float(h1)
        nrst = min((diff, diff-360, diff+360), key=abs)
        if direction is Direction.NEAREST or abs(nrst) <= underflow:
            return nrst
        elif direction is Direction.CLOCKWISE:
            return diff % 360
        elif direction is Direction.COUNTERCLOCKWISE:
            return diff if diff <= 0 else diff - 360

    def range(self, low_range, high_range=None):
        """Create a range relative to this heading.

        If high_range is not passed the range is centered on the current heading
        and extends equally in both directions from low_range.
        """
        return HeadingRange(
            self - low_range,
            self + (low_range if high_range is None else high_range))


class HeadingRange(Container):
    """Create a range of heading values, clockwise from min to max."""

    def __init__(self, min, max):
        self.min = min
        self.max = max

    def __contains__(self, x):
        wrap = self.min > self.max
        xgtmin = x >= self.min
        xltmax = x <= self.max
        return xgtmin or xltmax if wrap else xgtmin and xltmax

    def __repr__(self):
        return 'HeadingRange({}, {})'.format(self.min, self.max)