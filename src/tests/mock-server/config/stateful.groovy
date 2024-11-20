if (System.env.LOG_STATEFUL == '1') {
    logger.info("\n${context.request.body}\n")
}

respond {
    withStatusCode(200)
}
