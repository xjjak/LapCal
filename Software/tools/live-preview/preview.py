import serial

class LivePreview:
    def __init__(
            self,
            *pipes: Pipe
            port = "/dev/ttyUSB0",
            baudrate = 115200,
    ):
        self.serial = serial.Serial(port, baudrate)
        self.pipes = pipes
        self.output = None

    # def next_output(self):
    #     while not self.feature_extractor.available():
    #         line = self.serial.readline().decode()
    #         if self.output:
    #             self.feature_extractor.feed(line, output=self.output)
    #         else:
    #             self.feature_extractor.feed(line)

    #     features = self.feature_extractor.get_features()
    #     self.output = list(self.model.predict(features))
    #     return self.output

    def next(self):
        while not self.pipe.available():
            line = self.serial.readline().decode()
            if self.output:
                self.pipe.feed(line, output=self.output)
            else:
                self.pipe.feed(line)

        self.output = self.pipe.retrieve()
        return self.output

    def preview(self):
        print("Starting preview...")
        while True:
            print(self.next())


class MultiPreview:
    def __init__(
            self,
            feature_extractors,
            models,
            port = "/dev/ttyUSB0",
            baudrate = 115200,
    ):
        assert len(feature_extractors) == len(models)
        self.serial = serial.Serial(port, baudrate)
        self.feature_extractors = feature_extractors
        self.models = models
        self.output = [None]*len(models)

    def next_output(self):
        for index in range(len(self.models)):
            while not self.feature_extractors[index].available():
                line = self.serial.readline().decode()
                if self.output[index]:
                    self.feature_extractors[index].feed(line, output=self.output[index])
                else:
                    self.feature_extractors[index].feed(line)

            features = self.feature_extractors[index].get_features()
            self.output[index] = list(self.models[index].predict(features))
        return self.output

    def preview(self):
        print("Starting preview...")
        while True:
            print(self.next_output())
