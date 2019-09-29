package ru.lember.clocks

import ru.lember.clocks.view.ClocksView
import java.awt.BorderLayout
import java.awt.Dimension
import javax.swing.JFrame
import javax.swing.JPanel

fun main(args: Array<String>) {

//    val textArea = JTextArea()
//    textArea.text = "Hello, Kotlin/Swing world"
//    val scrollPane = JScrollPane(textArea)

    val panel = JPanel()
//    panel.preferredSize = Dimension(300, 300)
//    panel.isVisible = true
    val clocks = ClocksView()
    panel.add(clocks)

    val frame = JFrame("Hello, Kotlin/Swing")
    frame.contentPane.add(panel, BorderLayout.CENTER)
    frame.defaultCloseOperation = JFrame.EXIT_ON_CLOSE
    frame.size = Dimension(600, 400)
    frame.setLocationRelativeTo(null)
    frame.isVisible = true



}