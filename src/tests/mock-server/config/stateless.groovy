logger.info("stateless: ${context.request.body}")

respond {
    withStatusCode(200)
}
