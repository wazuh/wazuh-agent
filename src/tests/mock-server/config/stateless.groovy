if (System.env.LOG_STATELESS == '1') {
    logger.info("\n${context.request.body}\n")
}

respond {
    withStatusCode(200)
}
