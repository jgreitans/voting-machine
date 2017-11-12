using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using webapp.Locations.Interfaces;
using webapp.Locations.Services;
using webapp.Locations.Repositories;
using webapp.Votes.Interfaces;
using webapp.Votes.Services;
using webapp.Votes.Repositories;

namespace webapp
{
    public class Startup
    {
        public Startup(IConfiguration configuration)
        {
            Configuration = configuration;
        }

        public IConfiguration Configuration { get; }

        public void ConfigureServices(IServiceCollection services)
        {
            services.AddMvc();
            services.AddScoped<ILocationService, LocationService>();
            services.AddScoped<ILocationRepository, SqliteLocationRepository>();
            services.AddScoped<IVoteService, VoteService>();
            services.AddScoped<IVoteRepository, SqliteVoteRepository>();
        }

        public void Configure(IApplicationBuilder app, IHostingEnvironment env)
        {
            if (env.IsDevelopment())
            {
                app.UseDeveloperExceptionPage();
            }

            app.UseMvc();
        }
    }
}
