import groovy.json.JsonOutput
import java.net.URLDecoder
import java.net.URI

def mockFiles = [
    "invalidYaml.yml": "test:\n  invalid: true\n    invalid: true\n",
    "validYaml.yml": "test:\n  valid: true\n"
]

respond {
    try {
        def query = context.request.uri.split("\\?", 2)?.getAt(1)
        def queryParams = query?.split("&")?.collectEntries { param ->
            def (key, value) = param.split("=", 2)
            [(key): value ? URLDecoder.decode(value, "UTF-8") : null]
        }

        def fileName = queryParams?.get("file_name")

        if (fileName) {
            def fileContent = mockFiles[fileName]

            if (fileContent != null) {
                def fileBytes = fileContent.getBytes("UTF-8")
                def fileSize = fileBytes.length

                withStatusCode(200)
                withHeader("Content-Type", "application/octet-stream")
                withHeader("Content-Disposition", "attachment; filename=\"${fileName}\"")
                withHeader("Content-Length", fileSize.toString())
                withHeader("Connection", "close")
                withContent(fileContent)
            } else {
                withStatusCode(404)
                withContent(JsonOutput.toJson([error: "File not found"]))
            }
        } else {
            withStatusCode(400)
            withContent(JsonOutput.toJson([error: "Missing 'file_name' query parameter"]))
        }
    } catch (Exception e) {
        withStatusCode(500)
        logger.info("\n${e.message}\n")
    }
}
