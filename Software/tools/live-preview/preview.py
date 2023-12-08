import serial

class LivePreview:
    def __init__(
            self,
            feature_extractor,
            model,
            port = "/dev/ttyUSB0",
            baudrate = 115200,
    ):
        self.serial = serial.Serial(port, baudrate)
        self.feature_extractor = feature_extractor
        self.model = model
        self.output = None

    def next_output(self):
        while not self.feature_extractor.available():
            line = self.serial.readline().decode()
            if self.output:
                self.feature_extractor.feed(line, output=self.output)
            else:
                self.feature_extractor.feed(line)

        features = self.feature_extractor.get_features()
        self.output = self.model.predict(features)
        return self.output

    def preview(self):
        while True:
            print(self.next_output())
