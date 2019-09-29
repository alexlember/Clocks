package ru.lember.clocks.model;

import reactor.core.publisher.ReplayProcessor;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Clock {

    public static final ReplayProcessor<ClockTickEvent> PROCESSOR =
            ReplayProcessor.create();

    private boolean isRunning = false;

    private int currentSeconds = 0;
    private int currentMinutes = 0;
    private int currentHours = 0;

    private Mode mode = Mode.SETUP_HOURS;

    public Mode getMode() {
        return mode;
    }

    public Clock() {
        ExecutorService executorService = Executors.newSingleThreadExecutor();
        executorService.submit(() -> {
            while (true) {
                if (isRunning) {
                    try {
                        Thread.sleep(1000L);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    incrementSeconds();
                    broadcast();
                } else {
                    try {
                        Thread.sleep(5L);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        });
    }

    private String format(int digit) {
        String sDigit = String.valueOf(digit);
        if (sDigit.length() == 1) {
            sDigit = "0" + sDigit;
        }
        return sDigit;
    }

    private void stop() {
        System.out.println("clock stop");
        isRunning = false;
    }

    private void start() {
        System.out.println("clock start");
        isRunning = true;
    }

    public void switchSetup() {
        System.out.println("switchSetup");
        if (mode == Mode.SETUP_HOURS) {
            mode = Mode.SETUP_MINUTES;
            broadcast();
        } else if (mode == Mode.SETUP_MINUTES) {
            mode = Mode.SETUP_SECONDS;
            broadcast();
        } else if (mode == Mode.SETUP_SECONDS) {
            mode = Mode.RUNNING;
            start();
        } else {
            mode = Mode.SETUP_HOURS;
            stop();
            broadcast();
        }
    }

    public void increment() {
        if (mode == Mode.SETUP_HOURS) {
            incrementHours();
        } else if (mode == Mode.SETUP_MINUTES) {
            incrementMinutes();
        } else if (mode == Mode.SETUP_SECONDS) {
            incrementSeconds();
        }
        broadcast();
    }

    public void decrement() {
        if (mode == Mode.SETUP_HOURS) {
            decrementHours();
        } else if (mode == Mode.SETUP_MINUTES) {
            decrementMinutes();
        } else if (mode == Mode.SETUP_SECONDS) {
            decrementSeconds();
        }
        broadcast();
    }

    private void incrementHours() {
        if (currentHours == 23) {
            currentHours = 0;
        } else {
            currentHours++;
        }
        System.out.println("incrementHours. current hours: " + currentHours);
    }

    private void incrementMinutes() {
        if (currentMinutes == 59) {
            currentMinutes = 0;
            if (mode == Mode.RUNNING) {
                incrementHours();
            }
        } else {
            currentMinutes++;
        }
        System.out.println("incrementMinutes. current minutes: " + currentMinutes);
    }

    private void incrementSeconds() {
        if (currentSeconds == 59) {
            currentSeconds = 0;
            if (mode == Mode.RUNNING) {
                incrementMinutes();
            }
        } else {
            currentSeconds++;
        }
        System.out.println("incrementSeconds. current seconds: " + currentSeconds);
    }

    private void decrementHours() {
        if (currentHours == 0) {
            currentHours = 23;
        } else {
            currentHours--;
        }
        System.out.println("decrementHours. current hours: " + currentHours);
    }

    private void decrementMinutes() {
        if (currentMinutes == 0) {
            currentMinutes = 59;
        } else {
            currentMinutes--;
        }
        System.out.println("decrementMinutes. current minutes: " + currentMinutes);
    }

    private void decrementSeconds() {
        if (currentSeconds == 0) {
            currentSeconds = 59;
        } else {
            currentSeconds--;
        }
        System.out.println("decrementSeconds. current seconds: " + currentSeconds);
    }

    private void broadcast() {

        ClockTickEvent event = new ClockTickEvent(
                format(currentHours),
                format(currentMinutes),
                format(currentSeconds),
                mode
        );

        System.out.println("event: " + event);
        PROCESSOR.onNext(event);
    }

    public enum Mode {
        RUNNING,
        SETUP_HOURS,
        SETUP_MINUTES,
        SETUP_SECONDS
    }
}


