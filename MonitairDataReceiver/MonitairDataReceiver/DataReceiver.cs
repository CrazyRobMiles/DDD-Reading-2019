using IoTHubTrigger = Microsoft.Azure.WebJobs.ServiceBus.EventHubTriggerAttribute;

using Microsoft.Azure.WebJobs;
using Microsoft.Azure.WebJobs.Host;
using Microsoft.ServiceBus.Messaging;
using System.Text;
using System.Net.Http;
using System;

using Newtonsoft.Json;

namespace MonitairDataReceiver
{
    public class AirQReading 
    {
        // { "dev":"Monitair-15fb61","temp":25.87,"humidity":33.56,"pressure":1020.02,"timestamp":"1551281116"}

        [JsonProperty("dev")]
        public string PartitionKey { get; set; }
        [JsonProperty("timestamp")]
        public string RowKey { get; set; }
        [JsonProperty("PM10")]
        public float PM10 { get; set; }
        [JsonProperty("PM25")]
        public float PM25 { get; set; }
        [JsonProperty("temp")]
        public float Temp { get; set; }
        [JsonProperty("humidity")]
        public float Humidity { get; set; }
        [JsonProperty("pressure")]
        public float AirPress { get; set; }
        [JsonProperty("Lat")]
        public double Lattitude { get; set; }
        [JsonProperty("Long")]
        public double Longitude { get; set; }
    }

    public static class DataReceiver
    {
        private static HttpClient client = new HttpClient();

        [FunctionName("DataReceiver")]
        [return: Table("AirQualityReadings")]
        public static AirQReading Run([IoTHubTrigger("devices/#", Connection = "IoTHubConnectionString")]EventData message, TraceWriter log)
        {
            string readingJson = Encoding.UTF8.GetString(message.GetBytes());

            log.Info($"C# IoT Hub trigger function processed a message: {readingJson}");

            AirQReading result = new AirQReading();

            JsonConvert.PopulateObject(readingJson, result);

            return result;
        }
    }
}