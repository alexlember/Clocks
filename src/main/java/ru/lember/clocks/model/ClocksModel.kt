package ru.lember.clocks.model

import java.time.Clock

class ClocksModel(var clock: Clock, var mode: Mode = Mode.RUNNING_NO_SECONDS)