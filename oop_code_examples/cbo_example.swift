class Service {
    func doWork() { }
}

class Repository {
    func save() { }
}

class Logger {
    func log(_ message: String) { }
}

class CBOExample {
    var service: Service?

    var logger: Logger = Logger()

    init(service: Service?) {
        self.service = service
    }

    func setService(_ newService: Service) {
        service = newService
    }

    func createRepository() -> Repository {
        let repo = Repository()
        return repo
    }

    func work() {
        service?.doWork()
        logger.log("working")
    }
}
