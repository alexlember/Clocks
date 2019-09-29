package ru.lember.clocks.model

import lombok.extern.log4j.Log4j2

@Log4j2
class Try {

    internal fun subscribe() {

        val cancellation = Clock.PROCESSOR.subscribe(
                { e -> val s = "" },
                { t -> val s = "" })

    }
}
