package ru.lember.clocks.model

enum class Mode(val isRunning: Boolean) {

    RUNNING_NO_SECONDS(false),
    RUNNING_WITH_SECONDS(false),
    RUNNING_SECONDS_ON_DETECTION(false),

}