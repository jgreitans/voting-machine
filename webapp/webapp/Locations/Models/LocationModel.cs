using System;

namespace webapp.Locations.Models
{
    public class LocationModel
    {
        public LocationModel() {
        }

        public int Id { get; set; }
        public string Name { get; set; }
        public DateTime Ts { get; set; }
    }
}
