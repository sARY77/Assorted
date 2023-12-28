using System.Drawing.Imaging;
using System.Drawing;
using System.IO;
using Newtonsoft.Json;
using System.Net;
using System.Net.Http.Headers;
using System.Text;

namespace SeeWords
{
	public partial class Form1 : Form
	{
		const string endpoint = "seewords.cognitiveservices.azure.com";
		const string apiKey = "";
		const int maxSize = 1000;
		private class OcrResponse
		{
			public string? language { get; set; }
			public double? textAngle { get; set; }
			public string? orientation { get; set; }
			public OcrResponseRegion[]? regions { get; set; }
			public OcrResponseError? error { get; set; }
		}
		private class OcrResponseRegion
		{
			public string? boundingBox { get; set; }
			public OcrResponseLine[]? lines { get; set; }
		}
		private class OcrResponseLine
		{
			public string? boundingBox { get; set; }
			public OcrResponseWord[]? words { get; set; }
		}
		private class OcrResponseWord
		{
			public string? boundingBox { get; set; }
			public string? text { get; set; }
		}
		private class OcrResponseError
		{
			public string? code { get; set; }
			public string? message { get; set; }
		}
		public Form1()
		{
			InitializeComponent();
		}

		private string? SelectImage()
		{
			var openFileDialog = new OpenFileDialog
			{
				Filter = "JPEG files (*.jpg;*.jpeg)|*.jpg;*.jpeg",
				FilterIndex = 1,
				RestoreDirectory = true
			};

			if (openFileDialog.ShowDialog() == DialogResult.OK)
			{
				return openFileDialog.FileName;
			}

			return null;
		}

		public Bitmap LoadAndResizeBitmap(string imagePath)
		{
			var bitmap = new Bitmap(imagePath);
			if (bitmap.Width <= maxSize && bitmap.Height <= maxSize)
			{
				return bitmap;
			}

			double ratio = (double)bitmap.Width / bitmap.Height;
			int newWidth = ratio > 1 ? maxSize : (int)(maxSize * ratio);
			int newHeight = ratio > 1 ? (int)(maxSize / ratio) : maxSize;

			var newBitmap = new Bitmap(newWidth, newHeight);
			using (Graphics graphics = Graphics.FromImage(newBitmap))
			{
				graphics.DrawImage(bitmap, 0, 0, newWidth, newHeight);
			}

			return newBitmap;
		}

		private MemoryStream GetBitmapJpegStream(Bitmap bitmap)
		{
			var memoryStream = new MemoryStream();
			bitmap.Save(memoryStream, ImageFormat.Jpeg);
			return memoryStream;
		}

		private async Task<OcrResponse?> CallVisionApi(MemoryStream memoryStream)
		{
			using (var client = new HttpClient())
			{
				var request = new HttpRequestMessage(HttpMethod.Post, $"https://{endpoint}/vision/v3.2/ocr");
				request.Headers.Add("Ocp-Apim-Subscription-Key", apiKey);
				request.Content = new ByteArrayContent(memoryStream.ToArray());
				request.Content.Headers.ContentType = MediaTypeHeaderValue.Parse("application/octet-stream");

				var response = await client.SendAsync(request);
				string json = await response.Content.ReadAsStringAsync();
				return JsonConvert.DeserializeObject<OcrResponse>(json);
			}
		}

		private string ExtractWords(OcrResponse? ocrResponse)
		{
			var words = new StringBuilder();
			if (ocrResponse?.regions != null)
			{
				foreach (var region in ocrResponse.regions)
				{
					if (region?.lines != null)
					{
						foreach (var line in region.lines)
						{
							if (line?.words != null)
							{
								foreach (var word in line.words)
								{
									if (!string.IsNullOrWhiteSpace(word?.text))
									{
										if (words.Length > 0)
										{
											words.Append(" ");
										}
										words.Append(word.text.Trim());
									}
								}
							}
						}
					}
				}
			}
			return words.ToString();
		}

		private async void pictureBox1_Click(object sender, EventArgs e)
		{
			var imagePath = SelectImage();
			if (string.IsNullOrWhiteSpace(imagePath))
			{
				return;
			}
			using (var bitmap = LoadAndResizeBitmap(imagePath))
			{
				using (var memoryStream = GetBitmapJpegStream(bitmap))
				{
					pictureBox1.Image = new Bitmap(memoryStream);
					var ocrResponse = await CallVisionApi(memoryStream);
					textBox1.Text = ExtractWords(ocrResponse);
				}
			}
		}
	}
}
