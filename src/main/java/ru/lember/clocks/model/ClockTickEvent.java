package ru.lember.clocks.model;

import lombok.EqualsAndHashCode;
import lombok.Getter;
import lombok.ToString;

@Getter
@ToString
@EqualsAndHashCode
public class ClockTickEvent {

    public String hours;
    public String minutes;
    public String seconds;
    public Clock.Mode mode;

    public ClockTickEvent(
            String hours,
            String minutes,
            String seconds,
            Clock.Mode mode) {
        this.hours = hours;
        this.minutes = minutes;
        this.seconds = seconds;
        this.mode = mode;
    }
}
