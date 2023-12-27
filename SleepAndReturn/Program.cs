using System.Text;

var MyAllowSpecificOrigins = "_myAllowSpecificOrigins";

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddCors(options =>
{
    options.AddPolicy(name: MyAllowSpecificOrigins,
                      builder =>
                      {
						  builder.WithHeaders("*");
						  builder.WithMethods("*");
						  builder.WithOrigins("*");
                      });
});

// Add services to the container.

var app = builder.Build();

// Configure the HTTP request pipeline.

app.UseHttpsRedirection();

app.UseCors(MyAllowSpecificOrigins);

app.MapGet("/sleepAndReturn", (int numberOfMillisecondsToSleep, int numberOfBytesToReturn) =>
{
	Thread.Sleep(numberOfMillisecondsToSleep);
	var sb = new StringBuilder(numberOfBytesToReturn);
	for (int i = 0; i < numberOfBytesToReturn; ++i)
		sb.Append((char)('!' + Random.Shared.Next('~' - '!' + 1)));
	return sb.ToString();
});

app.Run();
