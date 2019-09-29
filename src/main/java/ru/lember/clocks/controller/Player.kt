package ru.lember.clocks.controller

import java.awt.event.MouseEvent
import java.awt.event.MouseListener

class Player : Runnable, MouseListener {

    private var holding: Boolean = false
    private var seconds: Int = 0
    private var thread: Thread? = null

    override fun mousePressed(e: MouseEvent) {
        holding = true
        thread = Thread(this)
        thread!!.start()
    }

    override fun mouseReleased(e: MouseEvent) {
        holding = false
        println("Held for: $seconds")
    }

    override fun mouseEntered(e: MouseEvent) {

    }

    override fun mouseExited(e: MouseEvent) {

    }

    override fun mouseClicked(e: MouseEvent) {}

    override fun run() {
        try {
            while (holding) {
                seconds++
                // put some code here
                if (seconds > 191490150) {
                    holding = false
                    println("Held for maximum time!")
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }


    }

}