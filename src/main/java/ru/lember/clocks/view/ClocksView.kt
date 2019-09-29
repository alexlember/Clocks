package ru.lember.clocks.view

import ru.lember.clocks.model.Clock
import ru.lember.clocks.model.ClockTickEvent
import ru.lember.clocks.model.Mode
import java.awt.Color
import java.awt.Dimension
import java.awt.event.ActionListener
import java.awt.event.MouseAdapter
import java.awt.event.MouseEvent
import java.util.concurrent.atomic.AtomicBoolean
import javax.swing.*
import javax.swing.border.LineBorder




class ClocksView : JComponent() {

    private val hours = JLabel("00")
    private val delimiter1 = JLabel(":")
    private val minutes = JLabel("00")
    private val delimiter2 = JLabel(":")
    private val seconds = JLabel("00")

    private val modeNoSeconds = JLabel("no sec")
    private val modeSecondsOn = JLabel("w sec")
    private val modeSecondsOnlyOnDetection = JLabel("detec")

    private val setupBtn = JButton("setup")
    private val increaseBtn = JButton("inc")
    private val decreaseBtn = JButton("dec")

    private val clock = Clock()
    private var mode = Mode.RUNNING_WITH_SECONDS

    var isLastHoursVisible: AtomicBoolean = AtomicBoolean(true)
    var isLastMinutesVisible: AtomicBoolean = AtomicBoolean(true)
    var isLastSecondsVisible: AtomicBoolean = AtomicBoolean(true)

    var hoursTimer: Timer = Timer(1000, ActionListener {
        if (isLastHoursVisible.get()) {
            isLastHoursVisible.set(false)
            hours.isVisible =  false
        } else {
            isLastHoursVisible.set(true)
            hours.isVisible =  true
        }
    })
    var minutesTimer: Timer = Timer(1000, ActionListener {
        if (isLastMinutesVisible.get()) {
            isLastMinutesVisible.set(false)
            minutes.isVisible =  false
        } else {
            isLastMinutesVisible.set(true)
            minutes.isVisible =  true
        }
    })
    var secondsTimer: Timer = Timer(1000, ActionListener {
        if (isLastSecondsVisible.get()) {
            isLastSecondsVisible.set(false)
            seconds.isVisible =  false
        } else {
            isLastSecondsVisible.set(true)
            seconds.isVisible =  true
        }
    })

    private fun stopAllTimers() {
        hoursTimer.stop()
        minutesTimer.stop()
        secondsTimer.stop()
        hours.isVisible = true
        minutes.isVisible = true
        seconds.isVisible = true
    }

    init {

        val cancellation = Clock.PROCESSOR.subscribe(
                { e ->
                    run {
                        if (e.mode == Clock.Mode.SETUP_MINUTES) {
                            hoursTimer.stop()
                            minutesTimer.start()
                            secondsTimer.stop()
                            hours.isVisible = true
                            seconds.isVisible = true
                        } else if (e.mode == Clock.Mode.SETUP_SECONDS) {
                            hoursTimer.stop()
                            minutesTimer.stop()
                            secondsTimer.start()
                            minutes.isVisible = true
                            hours.isVisible = true
                        } else if (e.mode == Clock.Mode.SETUP_HOURS) {
                            hoursTimer.start()
                            minutesTimer.stop()
                            secondsTimer.stop()
                            minutes.isVisible = true
                            seconds.isVisible = true
                        } else {
                            stopAllTimers()
                        }

                        printTime(e)
                    }
                },
                { t -> println("error: " + t.toString()) })


        val buttonPanel = JPanel()
        buttonPanel.add(setupBtn)
        buttonPanel.add(increaseBtn)
        buttonPanel.add(decreaseBtn)

        val ledPanel = JPanel()

        modeNoSeconds.border = LineBorder(Color.gray, 1, true)
        modeSecondsOn.border = LineBorder(Color.gray, 1, true)
        modeSecondsOnlyOnDetection.border = LineBorder(Color.gray, 1, true)

        ledPanel.add(modeNoSeconds)
        ledPanel.add(modeSecondsOn)
        ledPanel.add(modeSecondsOnlyOnDetection)

        val clockPanel = JPanel()
        clockPanel.add(hours)
        clockPanel.add(delimiter1)
        clockPanel.add(minutes)
        clockPanel.add(delimiter2)
        clockPanel.add(seconds)

        val detectionPanel = JPanel()
        detectionPanel.border = LineBorder(Color.gray, 1, true)
        detectionPanel.preferredSize = Dimension(100, 50)
        val label = JLabel("Detection panel. Put your mouse here")
        detectionPanel.add(label)

        this.add(buttonPanel)
        this.add(ledPanel)
        this.add(clockPanel)
        this.add(detectionPanel)

        this.preferredSize = Dimension(600, 400)
        this.border = LineBorder(Color.gray, 1, true)
        this.layout = BoxLayout(this, BoxLayout.Y_AXIS);

        setupBtn.addMouseListener(object : MouseAdapter() {
            override fun mouseClicked(e: MouseEvent) {

                if (e.clickCount == 2) {
                    if (clock.mode == Clock.Mode.RUNNING) {
                        println("change state to setup")
                        clock.switchSetup()
                    }
                } else if (e.clickCount == 1) {
                    if (clock.mode == Clock.Mode.RUNNING) {
                        println("change display mode")
                        switchDisplayMode()
                    } else {
                        println("change setup mode")
                        clock.switchSetup()
                    }
                }

            }
        })

        increaseBtn.addMouseListener(object : MouseAdapter() {
            override fun mouseClicked(e: MouseEvent) {

                if (clock.mode != Clock.Mode.RUNNING) {
                    println("increase btn click")
                    clock.increment()
                }

            }
        })

        decreaseBtn.addMouseListener(object : MouseAdapter() {
            override fun mouseClicked(e: MouseEvent) {

                if (clock.mode != Clock.Mode.RUNNING) {
                    println("decrease btn click")
                    clock.decrement()
                }

            }
        })

        switchDisplayMode()
    }

    private fun printTime(event: ClockTickEvent) {
        SwingUtilities.invokeLater {
            hours.text = event.hours
            minutes.text = event.minutes
            seconds.text = event.seconds

        }
    }

    private fun switchDisplayMode() {
        if (mode == Mode.RUNNING_WITH_SECONDS) {
            mode = Mode.RUNNING_SECONDS_ON_DETECTION
            seconds.isVisible = true
        } else if (mode == Mode.RUNNING_SECONDS_ON_DETECTION) {
            mode = Mode.RUNNING_NO_SECONDS
            seconds.isVisible = false
        } else {
            mode = Mode.RUNNING_WITH_SECONDS
            seconds.isVisible = true
        }
    }

}
