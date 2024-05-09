import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'firebase_options.dart';
import 'package:firebase_database/firebase_database.dart';
import 'dart:async';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp(
    options: DefaultFirebaseOptions.currentPlatform,
  );
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter-Firbase Realtime',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: const MyHomePage(title: 'SmartRoom'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  String temp = "", hum = "";
  String dist = "";

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
          title: const Text("Smart Room"),
          backgroundColor: Colors.blue,
          actions: [
            IconButton(onPressed: onPressed, icon: const Icon(Icons.thermostat))
          ]),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            TemperatureWidget(temperature: temp),
            const SizedBox(height: 40),
            HumidityWidget(humidity: hum),
            const SizedBox(height: 40),
            Container(width: 300,
              child: DistanceProgressBar(distance: dist)),
            Text("${dist}cm")
          ],
        ),
      ),
    );
  }

  void onPressed() {}

  void initState() {
    super.initState();
    // Start calling `_updateText` every 10 seconds
    Timer.periodic(Duration(seconds: 2), (timer) {
      _updateText();
    });
  }

  void _updateText() {
    // Your custom logic to update the text
    setState(() {
      loadData();
    });

    // Check if distance is 10 or less
    final double parsedDistance = double.tryParse(dist) ?? 0;
    if (parsedDistance <= 10) {
      _showOpenDoorDialog();
    }
  }

  Future<void> loadData() async {
    DatabaseReference ref = FirebaseDatabase.instance.ref('101');
    var event = await ref.once(DatabaseEventType.value);
    var data = event.snapshot.value;
    if (data is Map) {
      data.forEach((key, value) {
        print("$key : $value");
        if (key == "temp") {
          temp = value.toString();
        }
        if (key == "hum") {
          hum = value.toString();
        }
        if (key == "dist") {
          dist = value.toString();
        }
        setState(() {});
      });
    }
  }

  void _showOpenDoorDialog() {
    showDialog(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          title: const Text("Open the Door"),
          content: const Text("Someone is near!"),
          actions: <Widget>[
            TextButton(
              onPressed: () {
                Navigator.of(context).pop();
              },
              child: const Text("OK"),
            ),
          ],
        );
      },
    );
  }
}

class TemperatureWidget extends StatelessWidget {
  final String temperature;

  const TemperatureWidget({required this.temperature});

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        Icon(Icons.thermostat, size: 24),
        SizedBox(width: 8),
        Text(
          "$temperature °C",
          style: TextStyle(fontSize: 24),
        ),
      ],
    );
  }
}

class HumidityWidget extends StatelessWidget {
  final String humidity;

  const HumidityWidget({required this.humidity});

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        Icon(Icons.water_drop, size: 24),
        SizedBox(width: 8),
        Text(
          "$humidity %",
          style: TextStyle(fontSize: 24),
        ),
      ],
    );
  }
}

class DistanceProgressBar extends StatelessWidget {
  final String distance;

  const DistanceProgressBar({required this.distance});

  @override
  Widget build(BuildContext context) {
    final double parsedDistance = double.tryParse(distance) ?? 0;
    final double progress = 1.0 - (parsedDistance - 10) / (50 - 10);
    final double clampedProgress = progress.clamp(0.0, 1.0);

    return LinearProgressIndicator(
      value: clampedProgress,
      minHeight: 50,
      borderRadius: BorderRadius.circular(10),
      backgroundColor: Colors.grey[300],
      valueColor: AlwaysStoppedAnimation<Color>(
        parsedDistance <= 10 ? Colors.red : Colors.green,
      ),
    );
  }
}
