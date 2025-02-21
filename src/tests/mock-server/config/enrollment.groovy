logger.info("\n${context.request}\nBody: ${context.request.body}\n")

respond {
    withStatusCode(201)
}
